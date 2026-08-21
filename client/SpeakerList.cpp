/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "SpeakerList.h"

#include <cassert>
#include <algorithm>
#include <cstring>

#include <d3dx9.h>

#include <util/Storage.h>
#include <util/Logger.h>

#include <game/CPed.h>
#include <game/CSprite.h>
#include <game/CCamera.h>
#include <util/ImGuiUtil.h>
#include <util/GameUtil.h>
#include <util/Logger.h>
#include <util/Render.h>

#include "PluginConfig.h"

bool SpeakerList::Init(IDirect3DDevice9* const pDevice,
    const ResourceData& rSpeakerIcon, const ResourceData& rSpeakerFont) noexcept
{
    if (pDevice == nullptr)
        return false;

    if (SpeakerList::initStatus || !ImGuiUtil::IsInited())
        return false;

    try
    {
        SpeakerList::tSpeakerIcon = MakeTexture(pDevice, rSpeakerIcon);
    }
    catch (const std::exception& exception)
    {
        Logger::LogToFile("[sv:err:speakerlist:init] : failed to create speaker icon");
        SpeakerList::tSpeakerIcon.reset();
        return false;
    }

    Memory::ScopeExit iconResetScope { [] { SpeakerList::tSpeakerIcon.reset(); } };

    {
        float varFontSize { 0.f };

        if (!Render::ConvertBaseYValueToScreenYValue(kBaseFontSize, varFontSize))
        {
            Logger::LogToFile("[sv:err:speakerlist:init] : failed to convert font size");
            return false;
        }

        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;

        SpeakerList::pSpeakerFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(rSpeakerFont.ptr,
            static_cast<int>(rSpeakerFont.size), varFontSize, &fontConfig, ImGuiUtil::GetGlyphRanges());

        if (SpeakerList::pSpeakerFont == nullptr)
        {
            Logger::LogToFile("[sv:err:speakerlist:init] : failed to create speaker font");
            return false;
        }
    }

    if (!PluginConfig::IsSpeakerLoaded())
    {
        PluginConfig::SetSpeakerLoaded(true);
        SpeakerList::ResetConfigs();
    }

    SpeakerList::LoadIconTextures(pDevice);

    iconResetScope.Release();

    SpeakerList::initStatus = true;
    SpeakerList::SyncConfigs();

    return true;
}

void SpeakerList::Free() noexcept
{
    if (!SpeakerList::initStatus)
        return;

    for (auto& entry : SpeakerList::iconTextures)
    {
        if (entry.second != nullptr)
            entry.second->Release();
    }
    SpeakerList::iconTextures.clear();

    SpeakerList::tSpeakerIcon.reset();
    SpeakerList::pSpeakerFont = nullptr;

    SpeakerList::initStatus = false;
}

void SpeakerList::Show() noexcept
{
    SpeakerList::showStatus = true;
}

bool SpeakerList::IsShowed() noexcept
{
    return SpeakerList::showStatus;
}

void SpeakerList::Hide() noexcept
{
    SpeakerList::showStatus = false;
}

void SpeakerList::Render()
{
    if (!SpeakerList::initStatus || !SpeakerList::showStatus)
        return;

    const auto pNetGame = sv::RefNetGame();
    if (pNetGame == nullptr) return;

    const auto pPlayerPool = pNetGame->m_pPools->m_pPlayer;
    if (pPlayerPool == nullptr) return;

    float vLeftIndent { 0.f }, vScrWidth { 0.f }, vScrHeight { 0.f };

    if (!Render::ConvertBaseXValueToScreenXValue(kBaseLeftIndent, vLeftIndent)) return;
    if (!Render::GetScreenSize(vScrWidth, vScrHeight)) return;

    if (!ImGuiUtil::BeginRender()) return;

    const ImVec2 vWindowPadding { 4, 8 };
    const ImVec2 vFramePadding { 4, 8 };
    const ImVec2 vItemPadding { 4, 8 };

    ImGui::PushFont(SpeakerList::pSpeakerFont);

    const float vWidth = vScrWidth / 2.f - vLeftIndent;
    const float vHeight = 2.f * vWindowPadding.y + (kBaseLinesCount *
        (ImGui::GetTextLineHeight() + vFramePadding.y));

    const float vIconWidth = ImGui::GetTextLineHeight() + 5.f;
    const float vNickWidth = 0.2f * (vWidth - vIconWidth);
    const float vStreamsWidth = 0.8f * (vWidth - vIconWidth);

    const float vPosX = vLeftIndent;
    const float vPosY = (vScrHeight - vHeight) / 2.f;

    ImGui::SetNextWindowPos({ vPosX, vPosY });
    ImGui::SetNextWindowSize({ vWidth, vHeight });

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, vFramePadding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vWindowPadding);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, vItemPadding);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vItemPadding);

    if (ImGui::Begin("speakerListWindow", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoInputs))
    {
        ImGui::Columns(3, nullptr, false);

        ImGui::SetColumnWidth(0, vIconWidth);
        ImGui::SetColumnWidth(1, vNickWidth);
        ImGui::SetColumnWidth(2, vStreamsWidth);

        ImGui::SetColumnOffset(0, vWindowPadding.x);
        ImGui::SetColumnOffset(1, vWindowPadding.x + vIconWidth + vFramePadding.x);
        ImGui::SetColumnOffset(2, vWindowPadding.x + vIconWidth + vFramePadding.x + vNickWidth + vFramePadding.x);

        int curTextLine { 0 };

        for (WORD playerId { 0 }; playerId < sv::CPlayerPool::MAX_PLAYERS; ++playerId)
        {
            if (curTextLine >= kBaseLinesCount) break;

            if (pPlayerPool->m_pObject[playerId] == nullptr) continue;

            if (const auto playerName = pPlayerPool->m_pObject[playerId]->m_szNick.c_str(); playerName != nullptr)
            {
                if (!SpeakerList::playerStreams[playerId].empty())
                {
                    for (const auto& playerStream : SpeakerList::playerStreams[playerId])
                    {
                        if (playerStream.second.GetType() == StreamType::LocalStreamAtPlayer)
                        {
                            if (GameUtil::IsPlayerVisible(playerId))
                            {
                                if (const auto pPlayer = pPlayerPool->m_pObject[playerId]->m_pPlayer; pPlayer != nullptr)
                                {
                                    if (const auto pPlayerPed = pPlayer->m_pPed; pPlayerPed != nullptr)
                                    {
                                        if (const auto pGamePed = pPlayerPed->m_pGameEntity; pGamePed != nullptr)
                                        {
                                            const float distanceToCamera = (TheCamera.GetPosition() - pGamePed->GetPosition()).Magnitude();

                                            float vSpeakerIconSize { 0.f };

                                            if (Render::ConvertBaseYValueToScreenYValue(kBaseIconSize, vSpeakerIconSize))
                                            {
                                                vSpeakerIconSize *= PluginConfig::GetSpeakerIconScale();
                                                vSpeakerIconSize *= 5.f / distanceToCamera;

                                                float width, height;
                                                RwV3d playerPos, screenPos;

                                                static_cast<::CPed*>(pGamePed)->GetBonePosition(playerPos, 1, false);
                                                playerPos.z += 1.f;

                                                if (CSprite::CalcScreenCoors(playerPos, &screenPos, &width, &height, true, true))
                                                {
                                                    screenPos.x -= vSpeakerIconSize / 2.f;
                                                    screenPos.y -= vSpeakerIconSize / 2.f;

                                                    const float addX = PluginConfig::GetSpeakerIconOffsetX() * 5.f / distanceToCamera;
                                                    const float addY = PluginConfig::GetSpeakerIconOffsetY() * 5.f / distanceToCamera;

                                                    SpeakerList::tSpeakerIcon->Draw(screenPos.x + addX, screenPos.y + addY,
                                                        vSpeakerIconSize, vSpeakerIconSize, -1, 0.f);
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            break;
                        }
                    }

                    ImGui::PushID(playerId);

                    const auto alphaLevel = static_cast<DWORD>((1.f - static_cast<float>(curTextLine) /
                        static_cast<float>(kBaseLinesCount)) * 255.f) << 24;

                    const auto color = ImGui::ColorConvertU32ToFloat4(0x00ffffff | alphaLevel);

                    {
                        IDirect3DTexture9* iconTex = SpeakerList::tSpeakerIcon->GetTexture();
                        for (const auto& si : SpeakerList::playerStreams[playerId])
                        {
                            if (si.second.GetIconTexture() != nullptr)
                            {
                                iconTex = si.second.GetIconTexture();
                                break;
                            }
                        }
                        ImGui::Image(iconTex, { ImGui::GetTextLineHeight(),
                            ImGui::GetTextLineHeight() }, { 0, 0 }, { 1, 1 }, color);
                    }

                    ImGui::NextColumn();

                    ImGui::TextColored(color, "%s (%hu)", playerName, playerId);

                    ImGui::NextColumn();

                    for (const auto& streamInfo : SpeakerList::playerStreams[playerId])
                    {
                        if (streamInfo.second.GetColor() == NULL)
                            continue;

                        ImGui::PushID(&streamInfo);

                        const auto streamColor = ImGui::ColorConvertU32ToFloat4((streamInfo.second.GetColor() & 0x00ffffff) | alphaLevel);

                        ImGui::TextColored(streamColor, "[%s]", streamInfo.second.GetName().c_str());

                        ImGui::SameLine();
                        ImGui::PopID();
                    }

                    ImGui::NextColumn();
                    ImGui::PopID();

                    ++curTextLine;
                }
            }
        }

        ImGui::End();
    }

    ImGui::PopStyleVar(4);
    ImGui::PopFont();

    ImGuiUtil::EndRender();
}

int SpeakerList::GetSpeakerIconOffsetX() noexcept
{
    return PluginConfig::GetSpeakerIconOffsetX();
}

int SpeakerList::GetSpeakerIconOffsetY() noexcept
{
    return PluginConfig::GetSpeakerIconOffsetY();
}

float SpeakerList::GetSpeakerIconScale() noexcept
{
    return PluginConfig::GetSpeakerIconScale();
}

void SpeakerList::SetSpeakerIconOffsetX(const int speakerIconOffsetX) noexcept
{
    PluginConfig::SetSpeakerIconOffsetX(std::clamp(speakerIconOffsetX, -500, 500));
}

void SpeakerList::SetSpeakerIconOffsetY(const int speakerIconOffsetY) noexcept
{
    PluginConfig::SetSpeakerIconOffsetY(std::clamp(speakerIconOffsetY, -500, 500));
}

void SpeakerList::SetSpeakerIconScale(const float speakerIconScale) noexcept
{
    PluginConfig::SetSpeakerIconScale(std::clamp(speakerIconScale, 0.2f, 2.f));
}

void SpeakerList::SyncConfigs() noexcept
{
    SpeakerList::SetSpeakerIconOffsetX(PluginConfig::GetSpeakerIconOffsetX());
    SpeakerList::SetSpeakerIconOffsetY(PluginConfig::GetSpeakerIconOffsetY());
    SpeakerList::SetSpeakerIconScale(PluginConfig::GetSpeakerIconScale());
}

void SpeakerList::ResetConfigs() noexcept
{
    PluginConfig::SetSpeakerIconOffsetX(PluginConfig::kDefValSpeakerIconOffsetX);
    PluginConfig::SetSpeakerIconOffsetY(PluginConfig::kDefValSpeakerIconOffsetY);
    PluginConfig::SetSpeakerIconScale(PluginConfig::kDefValSpeakerIconScale);
}

void SpeakerList::UpdateIcon(const uint32_t streamId, const std::string& icon)
{
    Logger::LogToFile("[sv:dbg:speakerlist:updateicon] : stream(%u), icon(%s)",
        streamId, icon.c_str());

    SpeakerList::streamIconNames[streamId] = icon;
}

void SpeakerList::SetStreamIcon(Stream* const stream, IDirect3DTexture9* const texture)
{
    for (auto& playerEntry : SpeakerList::playerStreams)
    {
        for (auto& streamEntry : playerEntry)
        {
            if (streamEntry.first == stream)
            {
                streamEntry.second.iconTexture = texture;
            }
        }
    }
}

IDirect3DTexture9* SpeakerList::GetIconTexture(const std::string& name)
{
    const auto it = SpeakerList::iconTextures.find(name);
    return it != SpeakerList::iconTextures.end() ? it->second : nullptr;
}

std::map<std::string, IDirect3DTexture9*>& SpeakerList::GetIconTextures()
{
    return SpeakerList::iconTextures;
}

void SpeakerList::LoadIconTextures(IDirect3DDevice9* pDevice)
{
    Storage::ForEachFile([pDevice](const std::string& filePath) -> void
    {
        constexpr const char kPngExt[] = ".png";
        const size_t pathLen = filePath.size();
        if (pathLen > 4 && _stricmp(filePath.c_str() + pathLen - 4, kPngExt) == 0)
        {
            auto fileData = Storage::ReadFile(filePath);
            if (!fileData.empty())
            {
                IDirect3DTexture9* pTex = nullptr;
                if (SUCCEEDED(D3DXCreateTextureFromFileInMemory(pDevice,
                    fileData.data(), static_cast<UINT>(fileData.size()), &pTex)))
                {
                    size_t slashPos = filePath.find_last_of('\\');
                    size_t dotPos = filePath.find_last_of('.');
                    if (slashPos != std::string::npos && dotPos > slashPos)
                    {
                        std::string iconName = filePath.substr(slashPos + 1, dotPos - slashPos - 1);
                        SpeakerList::iconTextures[iconName] = pTex;
                        Logger::LogToFile("[sv:dbg:speakerlist:init] : loaded icon '%s' (tex:%p)",
                            iconName.c_str(), pTex);
                    }
                    else
                    {
                        pTex->Release();
                    }
                }
            }
        }
    });
}

void SpeakerList::OnSpeakerPlay(const Stream& stream, const WORD speaker) noexcept
{
    if (speaker != std::clamp<WORD>(speaker, 0, sv::CPlayerPool::MAX_PLAYERS - 1))
        return;

    SpeakerList::playerStreams[speaker][(Stream*)(&stream)] = stream.GetInfo();
}

void SpeakerList::OnSpeakerStop(const Stream& stream, const WORD speaker) noexcept
{
    if (speaker != std::clamp<WORD>(speaker, 0, sv::CPlayerPool::MAX_PLAYERS - 1))
        return;

    SpeakerList::playerStreams[speaker].erase((Stream*)(&stream));
}

bool SpeakerList::initStatus { false };
bool SpeakerList::showStatus { false };

ImFont* SpeakerList::pSpeakerFont { nullptr };
TexturePtr SpeakerList::tSpeakerIcon { nullptr };

std::array<std::unordered_map<Stream*, StreamInfo>, sv::CPlayerPool::MAX_PLAYERS> SpeakerList::playerStreams;

std::map<uint32_t, TexturePtr> SpeakerList::streamIcons;
std::map<uint32_t, std::string> SpeakerList::streamIconNames;
std::map<std::string, IDirect3DTexture9*> SpeakerList::iconTextures;
