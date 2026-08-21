/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <map>
#include <string>
#include <vector>

namespace Language {

    /*
        Every UI string the menu renders. Keys are matched against the
        "strings" object in the JSON language files.
    */
    enum class TextId {
        kTitleText,
        kTab1TitleText,
        kTab1Desc1TitleText,
        kTab1Desc1EnableSoundText,
        kTab1Desc1VolumeSoundText,
        kTab1Desc2TitleText,
        kTab1Desc2BalancerText,
        kTab1Desc2FilterText,
        kTab1Desc3TitleText,
        kTab1Desc3SpeakerIconScaleText,
        kTab1Desc3SpeakerIconOffsetXText,
        kTab1Desc3SpeakerIconOffsetYText,
        kTab1Desc4TitleText,
        kTab1Desc4ConfigResetText,
        kTab2TitleText,
        kTab2Desc1TitleText,
        kTab2Desc1EnableMicroText,
        kTab2Desc1MicroVolumeText,
        kTab2Desc1DeviceNameText,
        kTab2Desc1CheckDeviceText,
        kTab2Desc2TitleText,
        kTab2Desc2MicroIconScaleText,
        kTab2Desc2MicroIconPositionXText,
        kTab2Desc2MicroIconPositionYText,
        kTab2Desc2MicroIconMoveText,
        kTab2Desc3MicroNotFoundText,
        kTab3TitleText,
        kTab3Desc1TitleText,
        kTab3Desc1InputPlaceholderText,
        kTab3Desc2PlayerListText,
        kTab3Desc3BlackListText,
        kLanguageText,
        kCount
    };

    // Loads <languages>/<name>.json; falls back to built-in English on failure.
    void Load(const std::string& languageName) noexcept;

    // Returns the active display name of the current language (e.g. "Русский").
    const std::string& GetLanguageName() noexcept;

    // Display name for a given language id (file name), read from its JSON.
    // Falls back to the id itself when the file is missing or unreadable.
    std::string GetLanguageDisplayName(const std::string& languageId) noexcept;

    const char* Get(TextId id) noexcept;

    // Names of all available language files (without extension), for the menu combo.
    const std::vector<std::string>& GetAvailableLanguages() noexcept;

}
