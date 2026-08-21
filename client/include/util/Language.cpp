#include "Language.h"

#include <array>
#include <cstring>
#include <utility>

#include <Windows.h>

#include <json/json.hpp>

#include "Storage.h"
#include "Logger.h"

namespace {

    using Language::TextId;

    constexpr std::array<const char*, static_cast<std::size_t>(TextId::kCount)> kEnglishStrings =
    {
        /* kTitleText                       */ "Voice chat settings",
        /* kTab1TitleText                   */ "General",
        /* kTab1Desc1TitleText              */ "Sound",
        /* kTab1Desc1EnableSoundText        */ "Turn on sound",
        /* kTab1Desc1VolumeSoundText        */ "Sound volume",
        /* kTab1Desc2TitleText              */ "Effects",
        /* kTab1Desc2BalancerText           */ "Volume smoothing",
        /* kTab1Desc2FilterText             */ "High pass filter",
        /* kTab1Desc3TitleText              */ "Icon above players",
        /* kTab1Desc3SpeakerIconScaleText   */ "Scale",
        /* kTab1Desc3SpeakerIconOffsetXText */ "Offset by X",
        /* kTab1Desc3SpeakerIconOffsetYText */ "Offset by Y",
        /* kTab1Desc4TitleText              */ "Reset",
        /* kTab1Desc4ConfigResetText        */ "Reset all settings",
        /* kTab2TitleText                   */ "Microphone",
        /* kTab2Desc1TitleText              */ "Device",
        /* kTab2Desc1EnableMicroText        */ "Turn on microphone",
        /* kTab2Desc1MicroVolumeText        */ "Microphone volume",
        /* kTab2Desc1DeviceNameText         */ "Input device",
        /* kTab2Desc1CheckDeviceText        */ "Check device",
        /* kTab2Desc2TitleText              */ "Microphone icon",
        /* kTab2Desc2MicroIconScaleText     */ "Scale",
        /* kTab2Desc2MicroIconPositionXText */ "Position by X",
        /* kTab2Desc2MicroIconPositionYText */ "Position by Y",
        /* kTab2Desc2MicroIconMoveText      */ "Move",
        /* kTab2Desc3MicroNotFoundText      */ "No microphones available",
        /* kTab3TitleText                   */ "Black list",
        /* kTab3Desc1TitleText              */ "Filter",
        /* kTab3Desc1InputPlaceholderText   */ "Enter Player ID or Nickname...",
        /* kTab3Desc2PlayerListText         */ "Players online",
        /* kTab3Desc3BlackListText          */ "Blocked players",
        /* kLanguageText                    */ "Language",
    };

    constexpr std::array<const char*, static_cast<std::size_t>(TextId::kCount)> kTextKeys =
    {
        "title",
        "tab1.title",
        "tab1.desc1.title",
        "tab1.desc1.enable_sound",
        "tab1.desc1.volume_sound",
        "tab1.desc2.title",
        "tab1.desc2.balancer",
        "tab1.desc2.filter",
        "tab1.desc3.title",
        "tab1.desc3.speaker_icon_scale",
        "tab1.desc3.speaker_icon_offset_x",
        "tab1.desc3.speaker_icon_offset_y",
        "tab1.desc4.title",
        "tab1.desc4.config_reset",
        "tab2.title",
        "tab2.desc1.title",
        "tab2.desc1.enable_micro",
        "tab2.desc1.volume_micro",
        "tab2.desc1.device_name",
        "tab2.desc1.check_device",
        "tab2.desc2.title",
        "tab2.desc2.micro_icon_scale",
        "tab2.desc2.micro_icon_position_x",
        "tab2.desc2.micro_icon_position_y",
        "tab2.desc2.micro_icon_move",
        "tab2.desc3.micro_not_found",
        "tab3.title",
        "tab3.desc1.title",
        "tab3.desc1.input_placeholder",
        "tab3.desc2.player_list",
        "tab3.desc3.black_list",
        "language",
    };

    std::array<std::string, static_cast<std::size_t>(TextId::kCount)> activeStrings;
    std::string activeLanguageName { "English" };
    std::vector<std::string> availableLanguages;

    void ResetToEnglish() noexcept
    {
        for (std::size_t i { 0 }; i < kEnglishStrings.size(); ++i)
            activeStrings[i] = kEnglishStrings[i];
        activeLanguageName = "English";
    }

    // Lists every *.json file in <languages>/ (name only, no extension).
    void EnumerateLanguages() noexcept
    {
        availableLanguages.clear();

        const std::string languagesPath = Storage::GetLanguagesPath();
        if (languagesPath.empty()) return;

        const std::string searchPath = languagesPath + "*";
        WIN32_FIND_DATAA data;
        const HANDLE handle = FindFirstFileA(searchPath.c_str(), &data);
        if (handle == INVALID_HANDLE_VALUE) return;

        do
        {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                const std::string fileName = data.cFileName;
                constexpr const char kJsonExt[] = ".json";
                if (fileName.size() > 5 &&
                    _stricmp(fileName.c_str() + fileName.size() - 5, kJsonExt) == 0)
                {
                    availableLanguages.emplace_back(fileName.substr(0, fileName.size() - 5));
                }
            }
        }
        while (FindNextFileA(handle, &data));

        FindClose(handle);
    }

}

void Language::Load(const std::string& languageName) noexcept
{
    Language::ResetToEnglish();

    if (availableLanguages.empty())
        Language::EnumerateLanguages();

    if (languageName.empty())
        return;

    const std::string filePath = Storage::GetLanguagesPath() + languageName + ".json";
    const auto fileData = Storage::ReadFile(filePath);
    if (fileData.empty())
    {
        Logger::LogToFile("[sv:err:language] : failed to read language file '%s'", filePath.c_str());
        return;
    }

    try
    {
        const nlohmann::json root = nlohmann::json::parse(fileData.begin(), fileData.end());

        if (const auto it = root.find("language"); it != root.end() && it->is_string())
            activeLanguageName = it->get<std::string>();

        const auto& strings = root["strings"];
        if (strings.is_object())
        {
            for (std::size_t i { 0 }; i < kTextKeys.size(); ++i)
            {
                if (const auto it = strings.find(kTextKeys[i]);
                    it != strings.end() && it->is_string())
                {
                    activeStrings[i] = it->get<std::string>();
                }
            }
        }

        Logger::LogToFile("[sv:inf:language] : loaded language '%s' (%s)",
            activeLanguageName.c_str(), languageName.c_str());
    }
    catch (const std::exception& exception)
    {
        Logger::LogToFile("[sv:err:language] : failed to parse language file '%s' (%s)",
            filePath.c_str(), exception.what());
        Language::ResetToEnglish();
    }
}

const std::string& Language::GetLanguageName() noexcept
{
    return activeLanguageName;
}

const char* Language::Get(const TextId id) noexcept
{
    const auto index = static_cast<std::size_t>(id);
    if (index >= activeStrings.size())
        return "";

    return activeStrings[index].c_str();
}

const std::vector<std::string>& Language::GetAvailableLanguages() noexcept
{
    if (availableLanguages.empty())
        Language::EnumerateLanguages();

    return availableLanguages;
}
