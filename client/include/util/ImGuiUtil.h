/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <string>

#include <Windows.h>
#include <d3d9.h>

#include <imgui/imgui.h>

class ImGuiUtil {

    ImGuiUtil() = delete;
    ~ImGuiUtil() = delete;
    ImGuiUtil(const ImGuiUtil&) = delete;
    ImGuiUtil(ImGuiUtil&&) = delete;
    ImGuiUtil& operator=(const ImGuiUtil&) = delete;
    ImGuiUtil& operator=(ImGuiUtil&&) = delete;

public:

    static bool Init(IDirect3DDevice9* pDevice) noexcept;
    static bool IsInited() noexcept;
    static void Free() noexcept;

    static bool BeginRender() noexcept;
    static bool IsRendering() noexcept;
    static void EndRender() noexcept;

public:

    // Glyph ranges for UI fonts: Latin + Cyrillic + simplified Chinese.
    static const ImWchar* GetGlyphRanges() noexcept;

    // String helpers: SA-MP stores nicknames in the system ANSI code page
    // (e.g. GBK on Chinese Windows), while ImGui works with UTF-8.
    static std::string AnsiToUtf8(const char* ansi) noexcept;
    static std::string Utf8ToAnsi(const char* utf8) noexcept;

    // Case-insensitive substring search. Only ASCII letters are folded;
    // non-ASCII bytes (e.g. multi-byte ANSI nicknames) are compared verbatim
    // to avoid corrupting multi-byte sequences. Returns nullptr when not found.
    static const char* FindSubstringNoCase(const char* haystack, const char* needle) noexcept;

    static LRESULT WindowProc(HWND hWnd, UINT uMsg,
        WPARAM wParam, LPARAM lParam) noexcept;

private:

    static bool initStatus;
    static bool win32loadStatus;
    static bool dx9loadStatus;
    static bool renderStatus;

};
