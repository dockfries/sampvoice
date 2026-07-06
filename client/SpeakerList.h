/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <list>
#include <array>
#include <unordered_map>
#include <map>

#include <Windows.h>
#include <d3d9.h>
#include <rpc.h>
#include <d3dx9.h>

#include <imgui/imgui.h>

#include <svapi.h>
#include <util/Resource.h>
#include <util/Texture.h>

#include "Stream.h"

class SpeakerList {

    SpeakerList() = delete;
    ~SpeakerList() = delete;
    SpeakerList(const SpeakerList&) = delete;
    SpeakerList(SpeakerList&&) = delete;
    SpeakerList& operator=(const SpeakerList&) = delete;
    SpeakerList& operator=(SpeakerList&&) = delete;

private:

    static constexpr int kBaseLinesCount = 12;
    static constexpr float kBaseLeftIndent = 37.f;
    static constexpr float kBaseIconSize = 36.f;
    static constexpr float kBaseFontSize = 7.5f;

public:

    static bool Init(IDirect3DDevice9* pDevice,
        const Resource& rSpeakerIcon, const Resource& rSpeakerFont) noexcept;
    static void Free() noexcept;

    static void Show() noexcept;
    static bool IsShowed() noexcept;
    static void Hide() noexcept;

    static void Render();

    static int GetSpeakerIconOffsetX() noexcept;
    static int GetSpeakerIconOffsetY() noexcept;
    static float GetSpeakerIconScale() noexcept;

    static void SetSpeakerIconOffsetX(int speakerIconOffsetX) noexcept;
    static void SetSpeakerIconOffsetY(int speakerIconOffsetY) noexcept;
    static void SetSpeakerIconScale(float speakerIconScale) noexcept;

    static void SyncConfigs() noexcept;
    static void ResetConfigs() noexcept;

public:

    static void OnSpeakerPlay(const Stream& stream, WORD speaker) noexcept;
    static void OnSpeakerStop(const Stream& stream, WORD speaker) noexcept;

    static void UpdateIcon(uint32_t streamId, const std::string& icon);

    static void SetStreamIcon(Stream* stream, IDirect3DTexture9* texture);
    static IDirect3DTexture9* GetIconTexture(const std::string& name);
    static std::map<std::string, IDirect3DTexture9*>& GetIconTextures();

    static std::map<uint32_t, std::string> streamIconNames;
    static std::map<uint32_t, TexturePtr> streamIcons;

private:

    static bool initStatus;
    static bool showStatus;

    static ImFont* pSpeakerFont;
    static TexturePtr tSpeakerIcon;

    static std::array<std::unordered_map<Stream*, StreamInfo>, sv::CPlayerPool::MAX_PLAYERS> playerStreams;

    static std::map<std::string, IDirect3DTexture9*> iconTextures;

    static void LoadIconTextures(IDirect3DDevice9* pDevice);

};
