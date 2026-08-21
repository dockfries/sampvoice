/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "Plugin.h"

#include <cassert>

#include <CommCtrl.h>

#include <game/CRadar.h>
#include <game/CWorld.h>
#include <game/CSprite.h>
#include <util/Path.h>
#include <util/GameUtil.h>
#include <util/ImGuiUtil.h>
#include <util/KeyFilter.h>
#include <util/RakNet.h>
#include <util/Logger.h>
#include <util/Storage.h>
#include <util/Timer.h>

#include "Record.h"
#include "Playback.h"
#include "Network.h"
#include "BlackList.h"
#include "PluginConfig.h"
#include "MicroIcon.h"
#include "SpeakerList.h"
#include "PluginMenu.h"
#include "GlobalStream.h"
#include "StreamAtPoint.h"
#include "StreamAtVehicle.h"
#include "StreamAtPlayer.h"
#include "StreamAtObject.h"
#include "Header.h"

#pragma comment(lib, "comctl32.lib")

namespace
{
    /*
        Owns the bytes of an external resource file and exposes them as a
        ResourceData view, so UI modules can consume files from the
        <asi-dir>\resources\ directory.
    */
    struct ExternalResource {

        static ExternalResource Load(const char* const fileName) noexcept
        {
            ExternalResource result;
            result.data = Storage::ReadFile(Storage::GetResourcePath(fileName));
            if (!result.data.empty())
            {
                result.view.ptr = result.data.data();
                result.view.size = result.data.size();
            }
            return result;
        }

        bool IsValid() const noexcept
        {
            return view.IsValid();
        }

        std::vector<uint8_t> data;
        ResourceData view;
    };
}

bool Plugin::OnPluginLoad(const HMODULE hModule) noexcept
{
    if (hModule == nullptr)
        return false;

    Plugin::pModuleHandle = hModule;

    if (!Logger::Init(Path() / SV::kLogFileName))
        return false;

    if (!Storage::Initialize())
    {
        Logger::LogToFile("[sv:err:plugin] : failed to initialize storage module");
        Logger::Free();
        return false;
    }

    if (!Render::Init())
    {
        Logger::LogToFile("[sv:err:plugin] : failed to init render module");
        Logger::Free();
        return false;
    }

    Render::AddDeviceInitCallback(Plugin::OnDeviceInit);
    Render::AddBeforeResetCallback(Plugin::OnBeforeReset);
    Render::AddRenderCallback(Plugin::OnRender);
    Render::AddAfterResetCallback(Plugin::OnAfterReset);
    Render::AddDeviceFreeCallback(Plugin::OnDeviceFree);

    return true;
}

bool Plugin::OnSampLoad(const HMODULE hModule) noexcept
{
#if defined(SAMP_R3)
    Logger::LogToFile("[sv:dbg:samp:version] : client compiled for SA-MP 0.3.7-R3");
#elif defined(SAMP_R5)
    Logger::LogToFile("[sv:dbg:samp:version] : client compiled for SA-MP 0.3.7-R5");
#elif defined(SAMP_DL)
    Logger::LogToFile("[sv:dbg:samp:version] : client compiled for SA-MP 0.3.DL");
#else
    Logger::LogToFile("[sv:dbg:samp:version] : client compiled for SA-MP 0.3.7-R1");
#endif

    Plugin::sampBaseAddr = reinterpret_cast<DWORD>(hModule);

    if (!PluginConfig::Load(Path() / SV::kConfigFileName))
    {
        Logger::LogToFile("[sv:err:plugin] : failed to load configs");
    }

    if (!Samp::Init(Plugin::sampBaseAddr))
    {
        Logger::LogToFile("[sv:err:plugin] : failed to init samp");
        Render::Free();
        Logger::Free();
        return false;
    }

    Samp::AddLoadCallback(Plugin::OnInitGame);
    Samp::AddExitCallback(Plugin::OnExitGame);

    if (!Network::Init())
    {
        Logger::LogToFile("[sv:err:plugin] : failed to init network");
        Samp::Free();
        Render::Free();
        Logger::Free();
        return false;
    }

    Network::AddConnectCallback(Plugin::ConnectHandler);
    Network::AddSvConnectCallback(Plugin::PluginConnectHandler);
    Network::AddSvInitCallback(Plugin::PluginInitHandler);
    Network::AddDisconnectCallback(Plugin::DisconnectHandler);

    if (!Playback::Init())
    {
        Logger::LogToFile("[sv:err:plugin] : failed to init playback");
        Network::Free();
        Samp::Free();
        Render::Free();
        Logger::Free();
        return false;
    }

    return true;
}

void Plugin::OnInitGame() noexcept
{
    Plugin::drawRadarHook = MakeCallHook(0x58FC53, Plugin::DrawRadarHook);

    GameUtil::DisableAntiCheat();

    SpeakerList::Show();
    MicroIcon::Show();
}

void Plugin::OnExitGame() noexcept
{
    Network::Free();

    Plugin::streamTable.clear();

    Record::Free();
    Playback::Free();

    PluginConfig::Save(Path() / SV::kConfigFileName);
    BlackList::Free();

    Render::Free();
    Logger::Free();
}

void Plugin::MainLoop()
{
    if (!Samp::IsLoaded()) return;

    if (Plugin::gameStatus && !GameUtil::IsGameActive())
    {
        Logger::LogToFile("[sv:dbg:plugin] : game paused");

        for (const auto& stream : Plugin::streamTable)
            stream.second->Reset();

        KeyFilter::ReleaseAllKeys();
        Plugin::gameStatus = false;
    }
    else if (!Plugin::gameStatus && GameUtil::IsGameActive())
    {
        Logger::LogToFile("[sv:dbg:plugin] : game resumed");

        for (const auto& stream : Plugin::streamTable)
            stream.second->Reset();

        Plugin::gameStatus = true;
    }

    while (const auto controlPacket = Network::ReceiveControlPacket())
    {
        Plugin::ControlPacketHandler(*&*controlPacket);
    }

    while (const auto voicePacket = Network::ReceiveVoicePacket())
    {
        const auto& voicePacketRef = *voicePacket;

        if (BlackList::IsPlayerBlocked(voicePacketRef->sender))
            continue;

        const auto iter = Plugin::streamTable.find(voicePacketRef->stream);
        if (iter == Plugin::streamTable.end()) continue;

        iter->second->Push(*&voicePacketRef);
    }

    for (const auto& stream : Plugin::streamTable)
        stream.second->Tick();

    Playback::Tick();
    Record::Tick();

    KeyEvent keyEvent;

    while (KeyFilter::PopEvent(keyEvent))
    {
        if (keyEvent.isPressed)
        {
            if (!Record::GetMicroEnable()) continue;

            if (!Plugin::recordBusy && !Plugin::recordStatus && keyEvent.activeKeys == 1)
            {
                if (!Plugin::muteStatus)
                {
                    Plugin::recordStatus = true;
                    MicroIcon::SwitchToActiveIcon();
                    Record::StartRecording();
                }
                else
                {
                    MicroIcon::SwitchToMutedIcon();
                }
            }

            if (Plugin::muteStatus) continue;

            SV::PressKeyPacket pressKeyPacket {};

            pressKeyPacket.keyId = keyEvent.keyId;

            if (!Network::SendControlPacket(SV::ControlPacketType::pressKey, &pressKeyPacket, sizeof(pressKeyPacket)))
                Logger::LogToFile("[sv:err:main:HookWndProc] : failed to send PressKey packet");
        }
        else
        {
            if (!Record::GetMicroEnable()) continue;

            if (!Plugin::recordBusy && Plugin::recordStatus && !keyEvent.activeKeys)
            {
                MicroIcon::SwitchToPassiveIcon();
                Plugin::recordStatus = false;
            }

            if (Plugin::muteStatus) continue;

            SV::ReleaseKeyPacket releaseKeyPacket {};

            releaseKeyPacket.keyId = keyEvent.keyId;

            if (!Network::SendControlPacket(SV::ControlPacketType::releaseKey, &releaseKeyPacket, sizeof(releaseKeyPacket)))
                Logger::LogToFile("[sv:err:main:HookWndProc] : failed to send ReleaseKey packet");
        }
    }

    BYTE frameBuffer[Network::kMaxVoiceDataSize];

    if (const auto frameSize = Record::GetFrame(frameBuffer, sizeof(frameBuffer)))
    {
        if (!Network::SendVoicePacket(frameBuffer, frameSize))
            Logger::LogToFile("[sv:err:plugin] : failed to send voice packet");

        if (!Plugin::recordStatus)
        {
            Record::StopRecording();
            Network::EndSequence();
        }
    }
}

void Plugin::ConnectHandler(const std::string& serverIp, const WORD serverPort)
{
    Plugin::blacklistFilePath = static_cast<const std::string&>(Path() /
        "svblacklist_" + serverIp + "_" + std::to_string(serverPort) + ".txt");

    if (!BlackList::Load(Plugin::blacklistFilePath))
        Logger::LogToFile("[sv:err:plugin] : failed to open blacklist file");
    if (!BlackList::Init())
        Logger::LogToFile("[sv:err:plugin] : failed to init blacklist");
}

void Plugin::PluginConnectHandler(SV::ConnectPacket& connectStruct)
{
    connectStruct.signature = SV::kSignature;
    connectStruct.version = SV::kVersion;
    connectStruct.micro = Record::HasMicro();
}

bool Plugin::PluginInitHandler(const SV::PluginInitPacket& initPacket)
{
    Plugin::muteStatus = initPacket.mute;

    if (!Record::Init(initPacket.bitrate))
    {
        Logger::LogToFile("[sv:inf:plugin:packet:init] : failed init record");
    }

    return true;
}

void Plugin::ControlPacketHandler(const ControlPacket& controlPacket)
{
    switch (controlPacket.packet)
    {
        case SV::ControlPacketType::muteEnable:
        {
            if (controlPacket.length != 0) break;

            Logger::LogToFile("[sv:dbg:plugin:muteenable]");

            Plugin::muteStatus = true;
            Plugin::recordStatus = false;
            Plugin::recordBusy = false;

            KeyFilter::ReleaseAllKeys();
        } break;
        case SV::ControlPacketType::muteDisable:
        {
            if (controlPacket.length != 0) break;

            Logger::LogToFile("[sv:dbg:plugin:mutedisable]");

            Plugin::muteStatus = false;
        } break;
        case SV::ControlPacketType::startRecord:
        {
            if (controlPacket.length != 0) break;

            Logger::LogToFile("[sv:dbg:plugin:startrecord]");

            if (Plugin::muteStatus) break;

            Plugin::recordBusy = true;
            Plugin::recordStatus = true;

            Record::StartRecording();
        } break;
        case SV::ControlPacketType::stopRecord:
        {
            if (controlPacket.length != 0) break;

            Logger::LogToFile("[sv:dbg:plugin:stoprecord]");

            if (Plugin::muteStatus) break;

            Plugin::recordStatus = false;
            Plugin::recordBusy = false;
        } break;
        case SV::ControlPacketType::addKey:
        {
            const auto& stData = *reinterpret_cast<const SV::AddKeyPacket*>(controlPacket.data);
            if (controlPacket.length != sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:addkey] : keyid(0x%hhx)", stData.keyId);

            KeyFilter::AddKey(stData.keyId);
        } break;
        case SV::ControlPacketType::removeKey:
        {
            const auto& stData = *reinterpret_cast<const SV::RemoveKeyPacket*>(controlPacket.data);
            if (controlPacket.length != sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:removekey] : keyid(0x%hhx)", stData.keyId);

            KeyFilter::RemoveKey(stData.keyId);
        } break;
        case SV::ControlPacketType::removeAllKeys:
        {
            if (controlPacket.length) break;

            Logger::LogToFile("[sv:dbg:plugin:removeallkeys]");

            KeyFilter::RemoveAllKeys();
        } break;
        case SV::ControlPacketType::createGStream:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateGStreamPacket*>(controlPacket.data);
            if (controlPacket.length < sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:creategstream] : stream(%p), color(0x%x), name(%s)",
                stData.stream, stData.color, stData.color ? stData.name : "");

            const auto& streamPtr = Plugin::streamTable[stData.stream] =
                MakeGlobalStream(stData.color, stData.name);

            streamPtr->AddPlayCallback(SpeakerList::OnSpeakerPlay);
            streamPtr->AddStopCallback(SpeakerList::OnSpeakerStop);
        } break;
        case SV::ControlPacketType::createLPStream:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateLPStreamPacket*>(controlPacket.data);
            if (controlPacket.length < sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:createlpstream] : "
                "stream(%p), dist(%.2f), pos(%.2f;%.2f;%.2f), color(0x%x), name(%s)",
                stData.stream, stData.distance, stData.position.x, stData.position.y, stData.position.z,
                stData.color, stData.color ? stData.name : "");

            const auto& streamPtr = Plugin::streamTable[stData.stream] =
                MakeStreamAtPoint(stData.color, stData.name, stData.distance, stData.position);

            streamPtr->AddPlayCallback(SpeakerList::OnSpeakerPlay);
            streamPtr->AddStopCallback(SpeakerList::OnSpeakerStop);
        } break;
        case SV::ControlPacketType::createLStreamAtVehicle:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateLStreamAtPacket*>(controlPacket.data);
            if (controlPacket.length < sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:createlstreamatvehicle] : "
                "stream(%p), dist(%.2f), vehicle(%hu), color(0x%x), name(%s)",
                stData.stream, stData.distance, stData.target,
                stData.color, stData.color ? stData.name : "");

            const auto& streamPtr = Plugin::streamTable[stData.stream] =
                MakeStreamAtVehicle(stData.color, stData.name, stData.distance, stData.target);

            streamPtr->AddPlayCallback(SpeakerList::OnSpeakerPlay);
            streamPtr->AddStopCallback(SpeakerList::OnSpeakerStop);
        } break;
        case SV::ControlPacketType::createLStreamAtPlayer:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateLStreamAtPacket*>(controlPacket.data);
            if (controlPacket.length < sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:createlstreamatplayer] : "
                "stream(%p), dist(%.2f), player(%hu), color(0x%x), name(%s)",
                stData.stream, stData.distance, stData.target,
                stData.color, stData.color ? stData.name : "");

            const auto& streamPtr = Plugin::streamTable[stData.stream] =
                MakeStreamAtPlayer(stData.color, stData.name, stData.distance, stData.target);

            streamPtr->AddPlayCallback(SpeakerList::OnSpeakerPlay);
            streamPtr->AddStopCallback(SpeakerList::OnSpeakerStop);
        } break;
        case SV::ControlPacketType::createLStreamAtObject:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateLStreamAtPacket*>(controlPacket.data);
            if (controlPacket.length < sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:createlstreamatobject] : "
                "stream(%p), dist(%.2f), object(%hu), color(0x%x), name(%s)",
                stData.stream, stData.distance, stData.target,
                stData.color, stData.color ? stData.name : "");

            const auto& streamPtr = Plugin::streamTable[stData.stream] =
                MakeStreamAtObject(stData.color, stData.name, stData.distance, stData.target);

            streamPtr->AddPlayCallback(SpeakerList::OnSpeakerPlay);
            streamPtr->AddStopCallback(SpeakerList::OnSpeakerStop);
        } break;
        case SV::ControlPacketType::updateLStreamDistance:
        {
            const auto& stData = *reinterpret_cast<const SV::UpdateLStreamDistancePacket*>(controlPacket.data);
            if (controlPacket.length != sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:updatelpstreamdistance] : stream(%p), dist(%.2f)",
                stData.stream, stData.distance);

            const auto iter = Plugin::streamTable.find(stData.stream);
            if (iter == Plugin::streamTable.end()) break;

            static_cast<LocalStream*>(iter->second.get())->SetDistance(stData.distance);
        } break;
        case SV::ControlPacketType::updateLPStreamPosition:
        {
            const auto& stData = *reinterpret_cast<const SV::UpdateLPStreamPositionPacket*>(controlPacket.data);
            if (controlPacket.length != sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:updatelpstreamcoords] : stream(%p), pos(%.2f;%.2f;%.2f)",
                stData.stream, stData.position.x, stData.position.y, stData.position.z);

            const auto iter = Plugin::streamTable.find(stData.stream);
            if (iter == Plugin::streamTable.end()) break;

            static_cast<StreamAtPoint*>(iter->second.get())->SetPosition(stData.position);
        } break;
        case SV::ControlPacketType::deleteStream:
        {
            const auto& stData = *reinterpret_cast<const SV::DeleteStreamPacket*>(controlPacket.data);
            if (controlPacket.length != sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:deletestream] : stream(%p)", stData.stream);

            Plugin::streamTable.erase(stData.stream);
        } break;
        case SV::ControlPacketType::setStreamParameter:
        {
            const auto& stData = *reinterpret_cast<const SV::SetStreamParameterPacket*>(controlPacket.data);
            if (controlPacket.length != sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:streamsetparameter] : stream(%p), parameter(%hhu), value(%.2f)",
                stData.stream, stData.parameter, stData.value);

            const auto iter = Plugin::streamTable.find(stData.stream);
            if (iter == Plugin::streamTable.end()) break;

            iter->second->SetParameter(stData.parameter, stData.value);
        } break;
        case SV::ControlPacketType::slideStreamParameter:
        {
            const auto& stData = *reinterpret_cast<const SV::SlideStreamParameterPacket*>(controlPacket.data);
            if (controlPacket.length != sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:streamslideparameter] : "
                "stream(%p), parameter(%hhu), startvalue(%.2f), endvalue(%.2f), time(%u)",
                stData.stream, stData.parameter, stData.startvalue, stData.endvalue, stData.time);

            const auto iter = Plugin::streamTable.find(stData.stream);
            if (iter == Plugin::streamTable.end()) break;

            iter->second->SlideParameter(stData.parameter, stData.startvalue, stData.endvalue, stData.time);
        } break;
        case SV::ControlPacketType::createEffect:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateEffectPacket*>(controlPacket.data);
            if (controlPacket.length < sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:effectcreate] : "
                "stream(%p), effect(%p), number(%hhu), priority(%d)",
                stData.stream, stData.effect, stData.number, stData.priority);

            const auto iter = Plugin::streamTable.find(stData.stream);
            if (iter == Plugin::streamTable.end()) break;

            iter->second->EffectCreate(stData.effect, stData.number, stData.priority,
                stData.params, controlPacket.length - sizeof(stData));
        } break;
        case SV::ControlPacketType::deleteEffect:
        {
            const auto& stData = *reinterpret_cast<const SV::DeleteEffectPacket*>(controlPacket.data);
            if (controlPacket.length != sizeof(stData)) break;

            Logger::LogToFile("[sv:dbg:plugin:effectdelete] : stream(%p), effect(%p)",
                stData.stream, stData.effect);

            const auto iter = Plugin::streamTable.find(stData.stream);
            if (iter == Plugin::streamTable.end()) break;

            iter->second->EffectDelete(stData.effect);
        } break;
        case SV::ControlPacketType::setStreamIcon:
        {
            const auto& stData = *reinterpret_cast<const SV::SetStreamIconPacket*>(controlPacket.data);
            if (controlPacket.length < sizeof(stData)) break;

            const uint32_t iconLen = controlPacket.length - sizeof(stData);
            const std::string iconName(stData.name, iconLen > 0 ? iconLen - 1 : 0);

            Logger::LogToFile("[sv:dbg:plugin:setStreamIcon] : stream(%u), icon(%s)",
                stData.stream, iconName.c_str());

            SpeakerList::UpdateIcon(stData.stream, iconName);

            const auto streamIt = Plugin::streamTable.find(stData.stream);
            if (streamIt != Plugin::streamTable.end())
            {
                IDirect3DTexture9* iconTex = nullptr;
                if (!iconName.empty())
                    iconTex = SpeakerList::GetIconTexture(iconName);
                SpeakerList::SetStreamIcon(streamIt->second.get(), iconTex);
            }
        } break;
        case SV::ControlPacketType::appendFilter:
        {
            const auto& stData = *reinterpret_cast<const SV::AppendFilterPacket*>(controlPacket.data);
            if (controlPacket.length < sizeof(stData)) break;

            const auto iter = Plugin::streamTable.find(stData.stream);
            if (iter == Plugin::streamTable.end()) break;

            iter->second->EffectAppendFilter(stData.effect, stData.number, stData.priority,
                stData.params, controlPacket.length - sizeof(stData));
        } break;
        case SV::ControlPacketType::removeFilter:
        {
            const auto& stData = *reinterpret_cast<const SV::RemoveFilterPacket*>(controlPacket.data);
            if (controlPacket.length < sizeof(stData)) break;

            const auto iter = Plugin::streamTable.find(stData.stream);
            if (iter == Plugin::streamTable.end()) break;

            iter->second->EffectRemoveFilter(stData.effect, stData.number, stData.priority);
        } break;
        case SV::ControlPacketType::updateStreamTarget:
        {
            const auto& stData = *reinterpret_cast<const SV::UpdateStreamTargetPacket*>(controlPacket.data);
            if (controlPacket.length < sizeof(stData)) break;

            const auto iter = Plugin::streamTable.find(stData.stream);
            if (iter == Plugin::streamTable.end()) break;

            iter->second->SetTarget(stData.targetType, stData.targetId);
        } break;
    }
}

void Plugin::DisconnectHandler()
{
    Plugin::streamTable.clear();

    Plugin::muteStatus = false;
    Plugin::recordStatus = false;
    Plugin::recordBusy = false;

    if (!BlackList::Save(Plugin::blacklistFilePath))
        Logger::LogToFile("[sv:err:plugin] : failed to save blacklist file");

    BlackList::Free();
    Record::Free();
}

LRESULT CALLBACK Plugin::WindowProc(const HWND hWnd, const UINT uMsg,
    const WPARAM wParam, const LPARAM lParam, const UINT_PTR, const DWORD_PTR)
{
    if (PluginMenu::WindowProc(hWnd, uMsg, wParam, lParam))
        return TRUE;

    switch (uMsg)
    {
        case WM_KEYDOWN: KeyFilter::PushPressEvent(static_cast<BYTE>(wParam)); break;
        case WM_KEYUP: KeyFilter::PushReleaseEvent(static_cast<BYTE>(wParam)); break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void Plugin::OnDeviceInit(IDirect3D9* const pDirect,
                          IDirect3DDevice9* const pDevice,
                          const D3DPRESENT_PARAMETERS& dParameters)
{
    assert(pDirect);
    assert(pDevice);

    Logger::LogToFile("[sv:dbg:render:plugin] : graphics initialization started "
        "(direct:%p device:%p hwnd:%p)", pDirect, pDevice, dParameters.hDeviceWindow);

    const auto passiveIcon = ExternalResource::Load("micro_passive.png");
    const auto activeIcon = ExternalResource::Load("micro_active.png");
    const auto mutedIcon = ExternalResource::Load("micro_muted.png");
    const auto speakerIcon = ExternalResource::Load("speaker.png");
    const auto fontFile = ExternalResource::Load("font.ttf");
    const auto logoFile = ExternalResource::Load("logo.png");
    const auto shaderFile = ExternalResource::Load("gauss.hlsl");

    const auto microIconStatus = MicroIcon::Init(pDevice,
        passiveIcon.view, activeIcon.view, mutedIcon.view);
    Logger::LogToFile("[sv:dbg:render:plugin] : MicroIcon initialization %s",
        microIconStatus ? "succeeded" : "failed");

    const auto imguiStatus = ImGuiUtil::Init(pDevice);
    Logger::LogToFile("[sv:dbg:render:plugin] : ImGui initialization %s",
        imguiStatus ? "succeeded" : "failed");

    if (imguiStatus)
    {
        const auto speakerListStatus = SpeakerList::Init(pDevice,
            speakerIcon.view, fontFile.view);
        Logger::LogToFile("[sv:dbg:render:plugin] : SpeakerList initialization %s",
            speakerListStatus ? "succeeded" : "failed");

        const auto pluginMenuStatus = PluginMenu::Init(pDevice,
            shaderFile.view, logoFile.view, fontFile.view);
        Logger::LogToFile("[sv:dbg:render:plugin] : PluginMenu initialization %s",
            pluginMenuStatus ? "succeeded" : "failed");
    }
    else
    {
        Logger::LogToFile("[sv:err:render:plugin] : address-dependent UI initialization skipped "
            "(imgui:%d)", imguiStatus);
    }

    Plugin::origWndHandle = dParameters.hDeviceWindow;
    Plugin::windowSubclassStatus = Plugin::origWndHandle != nullptr &&
        SetWindowSubclass(Plugin::origWndHandle, Plugin::WindowProc,
            reinterpret_cast<UINT_PTR>(Plugin::pModuleHandle), 0) != FALSE;
    Logger::LogToFile("[sv:dbg:render:plugin] : window subclass installation %s",
        Plugin::windowSubclassStatus ? "succeeded" : "failed");

    Plugin::gameStatus = GameUtil::IsGameActive();
    Logger::LogToFile("[sv:dbg:render:plugin] : graphics initialization completed");
}

void Plugin::OnBeforeReset()
{
    Logger::LogToFile("[sv:dbg:render:plugin] : device reset cleanup started");

    PluginMenu::Free();
    Logger::LogToFile("[sv:dbg:render:plugin] : PluginMenu reset cleanup completed");

    SpeakerList::Free();
    Logger::LogToFile("[sv:dbg:render:plugin] : SpeakerList reset cleanup completed");

    ImGuiUtil::Free();
    Logger::LogToFile("[sv:dbg:render:plugin] : ImGui reset cleanup completed");

    MicroIcon::Free();
    Logger::LogToFile("[sv:dbg:render:plugin] : MicroIcon reset cleanup completed");

    Logger::LogToFile("[sv:dbg:render:plugin] : device reset cleanup completed");
}

void Plugin::OnRender()
{
    Timer::Tick();
    Plugin::MainLoop();
    MicroIcon::Update();
    PluginMenu::Update();
    PluginMenu::Render();
}

void Plugin::OnAfterReset(IDirect3DDevice9* const pDevice,
                          const D3DPRESENT_PARAMETERS& dParameters)
{
    assert(pDevice);

    Logger::LogToFile("[sv:dbg:render:plugin] : post-reset initialization started (device:%p)", pDevice);

    const auto passiveIcon = ExternalResource::Load("micro_passive.png");
    const auto activeIcon = ExternalResource::Load("micro_active.png");
    const auto mutedIcon = ExternalResource::Load("micro_muted.png");
    const auto speakerIcon = ExternalResource::Load("speaker.png");
    const auto fontFile = ExternalResource::Load("font.ttf");
    const auto logoFile = ExternalResource::Load("logo.png");
    const auto shaderFile = ExternalResource::Load("gauss.hlsl");

    const auto microIconStatus = MicroIcon::Init(pDevice,
        passiveIcon.view, activeIcon.view, mutedIcon.view);
    Logger::LogToFile("[sv:dbg:render:plugin] : post-reset MicroIcon initialization %s",
        microIconStatus ? "succeeded" : "failed");

    const auto imguiStatus = ImGuiUtil::Init(pDevice);
    Logger::LogToFile("[sv:dbg:render:plugin] : post-reset ImGui initialization %s",
        imguiStatus ? "succeeded" : "failed");

    if (imguiStatus)
    {
        const auto speakerListStatus = SpeakerList::Init(pDevice,
            speakerIcon.view, fontFile.view);
        Logger::LogToFile("[sv:dbg:render:plugin] : post-reset SpeakerList initialization %s",
            speakerListStatus ? "succeeded" : "failed");

        const auto pluginMenuStatus = PluginMenu::Init(pDevice,
            shaderFile.view, logoFile.view, fontFile.view);
        Logger::LogToFile("[sv:dbg:render:plugin] : post-reset PluginMenu initialization %s",
            pluginMenuStatus ? "succeeded" : "failed");
    }
    else
    {
        Logger::LogToFile("[sv:err:render:plugin] : post-reset address-dependent UI initialization skipped "
            "(imgui:%d)", imguiStatus);
    }

    Plugin::gameStatus = GameUtil::IsGameActive();
    Logger::LogToFile("[sv:dbg:render:plugin] : post-reset initialization completed");
}

void Plugin::OnDeviceFree()
{
    Logger::LogToFile("[sv:dbg:render:plugin] : device release cleanup started");

    if (Plugin::windowSubclassStatus && Plugin::origWndHandle != nullptr)
    {
        RemoveWindowSubclass(Plugin::origWndHandle, Plugin::WindowProc,
            reinterpret_cast<UINT_PTR>(Plugin::pModuleHandle));
    }

    Plugin::windowSubclassStatus = false;
    Plugin::origWndHandle = nullptr;

    PluginMenu::Free();
    SpeakerList::Free();
    ImGuiUtil::Free();
    MicroIcon::Free();

    Logger::LogToFile("[sv:dbg:render:plugin] : device release cleanup completed");
}

void Plugin::DrawRadarHook()
{
    static_cast<void(*)()>(Plugin::drawRadarHook->callFuncAddr)();

    SpeakerList::Render();
    MicroIcon::Render();
}

HMODULE Plugin::pModuleHandle { NULL };
DWORD Plugin::sampBaseAddr { 0 };

bool Plugin::muteStatus { false };
bool Plugin::recordStatus { false };
bool Plugin::recordBusy { false };

std::map<DWORD, StreamPtr> Plugin::streamTable;
std::string Plugin::blacklistFilePath;
bool Plugin::gameStatus { false };

HWND Plugin::origWndHandle { NULL };
bool Plugin::windowSubclassStatus { false };

Memory::CallHookPtr Plugin::drawRadarHook { nullptr };
