/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <array>

#include <Windows.h>
#include <d3d9.h>

#include <imgui/imgui.h>
#include <util/Memory.hpp>
#include <util/Language.h>
#include <util/ResourceData.h>
#include <svapi.h>
#include <util/BlurEffect.h>
#include <util/Texture.h>
#include <util/Render.h>

class PluginMenu {

    PluginMenu() = delete;
    ~PluginMenu() = delete;
    PluginMenu(const PluginMenu&) = delete;
    PluginMenu(PluginMenu&&) = delete;
    PluginMenu& operator=(const PluginMenu&) = delete;
    PluginMenu& operator=(PluginMenu&&) = delete;

private:

    // UI strings resolve through the runtime language table.
#define kTitleText                       Language::Get(Language::TextId::kTitleText)
#define kTab1TitleText                   Language::Get(Language::TextId::kTab1TitleText)
#define kTab1Desc1TitleText              Language::Get(Language::TextId::kTab1Desc1TitleText)
#define kTab1Desc1EnableSoundText        Language::Get(Language::TextId::kTab1Desc1EnableSoundText)
#define kTab1Desc1VolumeSoundText        Language::Get(Language::TextId::kTab1Desc1VolumeSoundText)
#define kTab1Desc2TitleText              Language::Get(Language::TextId::kTab1Desc2TitleText)
#define kTab1Desc2BalancerText           Language::Get(Language::TextId::kTab1Desc2BalancerText)
#define kTab1Desc2FilterText             Language::Get(Language::TextId::kTab1Desc2FilterText)
#define kTab1Desc3TitleText              Language::Get(Language::TextId::kTab1Desc3TitleText)
#define kTab1Desc3SpeakerIconScaleText   Language::Get(Language::TextId::kTab1Desc3SpeakerIconScaleText)
#define kTab1Desc3SpeakerIconOffsetXText Language::Get(Language::TextId::kTab1Desc3SpeakerIconOffsetXText)
#define kTab1Desc3SpeakerIconOffsetYText Language::Get(Language::TextId::kTab1Desc3SpeakerIconOffsetYText)
#define kTab1Desc4TitleText              Language::Get(Language::TextId::kTab1Desc4TitleText)
#define kTab1Desc4ConfigResetText        Language::Get(Language::TextId::kTab1Desc4ConfigResetText)
#define kTab2TitleText                   Language::Get(Language::TextId::kTab2TitleText)
#define kTab2Desc1TitleText              Language::Get(Language::TextId::kTab2Desc1TitleText)
#define kTab2Desc1EnableMicroText        Language::Get(Language::TextId::kTab2Desc1EnableMicroText)
#define kTab2Desc1MicroVolumeText        Language::Get(Language::TextId::kTab2Desc1MicroVolumeText)
#define kTab2Desc1DeviceNameText         Language::Get(Language::TextId::kTab2Desc1DeviceNameText)
#define kTab2Desc1CheckDeviceText        Language::Get(Language::TextId::kTab2Desc1CheckDeviceText)
#define kTab2Desc2TitleText              Language::Get(Language::TextId::kTab2Desc2TitleText)
#define kTab2Desc2MicroIconScaleText     Language::Get(Language::TextId::kTab2Desc2MicroIconScaleText)
#define kTab2Desc2MicroIconPositionXText Language::Get(Language::TextId::kTab2Desc2MicroIconPositionXText)
#define kTab2Desc2MicroIconPositionYText Language::Get(Language::TextId::kTab2Desc2MicroIconPositionYText)
#define kTab2Desc2MicroIconMoveText      Language::Get(Language::TextId::kTab2Desc2MicroIconMoveText)
#define kTab2Desc3MicroNotFoundText      Language::Get(Language::TextId::kTab2Desc3MicroNotFoundText)
#define kTab3TitleText                   Language::Get(Language::TextId::kTab3TitleText)
#define kTab3Desc1TitleText              Language::Get(Language::TextId::kTab3Desc1TitleText)
#define kTab3Desc1InputPlaceholderText   Language::Get(Language::TextId::kTab3Desc1InputPlaceholderText)
#define kTab3Desc2PlayerListText         Language::Get(Language::TextId::kTab3Desc2PlayerListText)
#define kTab3Desc3BlackListText          Language::Get(Language::TextId::kTab3Desc3BlackListText)
#define kLanguageText                    Language::Get(Language::TextId::kLanguageText)

    static constexpr float kBaseMenuWidth                  = 0.6f * Render::kBaseWidth;
    static constexpr float kBaseMenuHeight                 = 0.7f * Render::kBaseHeight;
    static constexpr float kBaseMenuPaddingX               = 20.f;
    static constexpr float kBaseMenuPaddingY               = 10.f;
    static constexpr float kBaseMenuFramePaddingX          = 10.f;
    static constexpr float kBaseMenuFramePaddingY          = 0.5f;
    static constexpr float kBaseMenuItemSpacingX           = 20.f;
    static constexpr float kBaseMenuItemSpacingY           = 2.f;
    static constexpr float kBaseMenuItemInnerSpacingX      = 10.f;
    static constexpr float kBaseMenuItemInnerSpacingY      = 10.f;
    static constexpr float kBaseMenuRounding               = 10.f;
    static constexpr float kBaseFontTitleSize              = 20.f;
    static constexpr float kBaseFontTabSize                = 14.f;
    static constexpr float kBaseFontDescSize               = 12.f;
    static constexpr float kBaseFontSize                   = 10.f;
    static constexpr int   kTabsCount                      = 3;
    static constexpr float kBaseTabPadding                 = 4.f;
    static constexpr float kBaseTabWidth                   = (kBaseMenuWidth - (2 * kBaseMenuPaddingX +
                                                             (kTabsCount - 1) * kBaseTabPadding)) / kTabsCount;
    static constexpr float kBaseTabHeight                  = kBaseTabWidth / 6.f;
    static constexpr float kBlurLevelIncrement             = 5.f;
    static constexpr float kBlurLevelDecrement             = -5.f;

public:

    static bool Init(IDirect3DDevice9* pDevice,
        const ResourceData& rShader, const ResourceData& rLogo, const ResourceData& rFont) noexcept;
    static void Free() noexcept;

    static bool Show() noexcept;
    static bool IsShowed() noexcept;
    static void Hide() noexcept;

    static void Render() noexcept;
    static void Update() noexcept;

    static LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

private:

    static void SyncOptions() noexcept;

private:

    static bool initStatus;
    static bool showStatus;

    static float blurLevel;
    static float blurLevelDeviation;
    static BlurEffectPtr blurEffect;

    static TexturePtr tLogo;

    static ImFont* pTitleFont;
    static ImFont* pTabFont;
    static ImFont* pDescFont;
    static ImFont* pDefFont;

    static Memory::PatchPtr openChatFuncPatch;
    static Memory::PatchPtr openScoreboardFuncPatch;
    static Memory::PatchPtr switchModeFuncPatch;

    static int prevChatMode;

    // Configs
    // ------------------------------------------------------------------------------------------

    static bool soundEnable;
    static int soundVolume;
    static bool soundBalancer;
    static bool soundFilter;

    static float speakerIconScale;
    static int speakerIconOffsetX;
    static int speakerIconOffsetY;

    static bool microEnable;
    static int microVolume;
    static int deviceIndex;

    static float microIconScale;
    static int microIconPositionX;
    static int microIconPositionY;
    static D3DCOLOR microIconColor;
    static float microIconAngle;

    // Internal options
    // ------------------------------------------------------------------------------------------

    static int iSelectedMenu;
    static bool bCheckDevice;
    static bool bMicroMovement;
    static std::array<char, 64> nBuffer;

};
