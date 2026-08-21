/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "ImGuiUtil.h"

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx9.h>
#include <imgui/imgui_impl_win32.h>

#include "Memory.hpp"
#include "Logger.h"

bool ImGuiUtil::Init(IDirect3DDevice9* const pDevice) noexcept
{
    assert(pDevice != nullptr);

    if (ImGuiUtil::initStatus)
        return false;

    HWND hDeviceWindow { nullptr };
    {
        IDirect3DSwapChain9* pSwapChain { nullptr };

        if (const auto hResult = pDevice->GetSwapChain(0, &pSwapChain); FAILED(hResult))
        {
            Logger::LogToFile("[err:imgui:init] : failed to get swap chain (code:%ld)", hResult);
            return false;
        }

        const Memory::ScopeExit scope { [&pSwapChain] { pSwapChain->Release(); } };

        D3DPRESENT_PARAMETERS dParameters {};

        if (const auto hResult = pSwapChain->GetPresentParameters(&dParameters); FAILED(hResult))
        {
            Logger::LogToFile("[err:imgui:init] : failed to get present parameters (code:%ld)", hResult);
            return false;
        }

        hDeviceWindow = dParameters.hDeviceWindow;
    }

    IMGUI_CHECKVERSION();
    if (ImGui::CreateContext() == nullptr)
    {
        Logger::LogToFile("[sv:err:imgui:init] : failed to create ImGui context");
        return false;
    }

    ImGui::GetIO().IniFilename = NULL;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGuiUtil::win32loadStatus = ImGui_ImplWin32_Init(hDeviceWindow);
    if (!ImGuiUtil::win32loadStatus)
    {
        Logger::LogToFile("[sv:err:imgui:init] : failed to initialize Win32 backend");
        ImGui::DestroyContext();
        return false;
    }

    ImGuiUtil::dx9loadStatus = ImGui_ImplDX9_Init(pDevice);
    if (!ImGuiUtil::dx9loadStatus)
    {
        Logger::LogToFile("[sv:err:imgui:init] : failed to initialize Direct3D9 backend");
        ImGui_ImplWin32_Shutdown();
        ImGuiUtil::win32loadStatus = false;
        ImGui::DestroyContext();
        return false;
    }

    ImGuiUtil::renderStatus = false;
    ImGuiUtil::initStatus = true;

    return true;
}

bool ImGuiUtil::IsInited() noexcept
{
    return ImGuiUtil::initStatus;
}

void ImGuiUtil::Free() noexcept
{
    ImGuiUtil::EndRender();

    if (ImGuiUtil::dx9loadStatus)
        ImGui_ImplDX9_Shutdown();

    ImGuiUtil::dx9loadStatus = false;

    if (ImGuiUtil::win32loadStatus)
        ImGui_ImplWin32_Shutdown();

    ImGuiUtil::win32loadStatus = false;

    if (ImGuiUtil::initStatus)
        ImGui::DestroyContext();

    ImGuiUtil::initStatus = false;
}

bool ImGuiUtil::BeginRender() noexcept
{
    if (!ImGuiUtil::initStatus)
        return false;

    if (ImGuiUtil::renderStatus)
        return false;

    if (ImGuiUtil::dx9loadStatus)
        ImGui_ImplDX9_NewFrame();

    if (ImGuiUtil::win32loadStatus)
        ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    ImGuiUtil::renderStatus = true;

    return true;
}

bool ImGuiUtil::IsRendering() noexcept
{
    return ImGuiUtil::renderStatus;
}

void ImGuiUtil::EndRender() noexcept
{
    if (!ImGuiUtil::initStatus)
        return;

    if (!ImGuiUtil::renderStatus)
        return;

    ImGui::EndFrame();
    ImGui::Render();

    if (ImGuiUtil::dx9loadStatus)
    {
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    ImGuiUtil::renderStatus = false;
}

LRESULT ImGuiUtil::WindowProc(const HWND hWnd, const UINT uMsg,
                              const WPARAM wParam, const LPARAM lParam) noexcept
{
    if (!ImGuiUtil::initStatus)
        return FALSE;

    if (!ImGuiUtil::win32loadStatus)
        return FALSE;

    if (uMsg == WM_CHAR)
    {
        // wParam carries a UTF-16 code unit on Unicode windows (the SA:MP
        // window is registered as Unicode, and Microsoft Pinyin IME delivers
        // UTF-16 here). On ANSI/MBCS windows it is a single-byte or DBCS
        // value, which needs code-page conversion.
        if (::IsWindowUnicode(hWnd))
        {
            if (wParam > 0 && wParam < 0x10000)
                ImGui::GetIO().AddInputCharacter(static_cast<ImWchar>(wParam));
        }
        else
        {
            wchar_t wch { 0 };
            if (::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                reinterpret_cast<const char*>(&wParam), 2, &wch, 1) > 0)
            {
                ImGui::GetIO().AddInputCharacter(wch);
            }
        }

        return TRUE;
    }

    if (uMsg == WM_IME_CHAR)
    {
        // On Unicode windows wParam is already UTF-16; on ANSI windows it
        // packs the DBCS lead/trail bytes and needs code-page conversion.
        if (::IsWindowUnicode(hWnd))
        {
            if (wParam > 0 && wParam < 0x10000)
                ImGui::GetIO().AddInputCharacter(static_cast<ImWchar>(wParam));
        }
        else
        {
            WPARAM dbcs = wParam;
            if (::IsDBCSLeadByte(HIBYTE(dbcs)))
                dbcs = MAKEWORD(HIBYTE(dbcs), LOBYTE(dbcs));

            wchar_t wch { 0 };
            if (::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                reinterpret_cast<const char*>(&dbcs), 2, &wch, 1) > 0)
            {
                ImGui::GetIO().AddInputCharacter(wch);
            }
        }

        return TRUE;
    }

    extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
    return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
}

const ImWchar* ImGuiUtil::GetGlyphRanges() noexcept
{
    static const ImWchar* ranges = []() -> const ImWchar*
    {
        ImFontGlyphRangesBuilder builder;

        // Latin + Latin-1 + Cyrillic
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
        // Full CJK ideographs + Hiragana/Katakana + half-width
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
        // Korean Hangul
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesKorean());
        // Thai
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesThai());

        // Additional blocks commonly found in player nicknames:
        // Latin Extended-A/B, IPA, Greek, Cyrillic Supplement, Hebrew,
        // Arabic (glyphs only; ImGui has no RTL shaping), Latin Extended Additional.
        static const ImWchar kExtraRanges[] =
        {
            0x0100, 0x02AF,  // Latin Extended-A/B, IPA Extensions
            0x0370, 0x03FF,  // Greek and Coptic
            0x0500, 0x052F,  // Cyrillic Supplement
            0x0590, 0x05FF,  // Hebrew
            0x0600, 0x06FF,  // Arabic
            0x1E00, 0x1EFF,  // Latin Extended Additional
            0
        };
        builder.AddRanges(kExtraRanges);

        static ImVector<ImWchar> result;
        builder.BuildRanges(&result);
        return result.Data;
    }();

    return ranges;
}

std::string ImGuiUtil::AnsiToUtf8(const char* const ansi) noexcept
{
    if (ansi == nullptr || *ansi == '\0')
        return {};

    const int wideLen = MultiByteToWideChar(CP_ACP, 0, ansi, -1, nullptr, 0);
    if (wideLen <= 0)
        return ansi;

    std::vector<wchar_t> wideBuf(wideLen);
    MultiByteToWideChar(CP_ACP, 0, ansi, -1, wideBuf.data(), wideLen);

    const int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideBuf.data(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Len <= 0)
        return ansi;

    std::string utf8Buf(utf8Len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wideBuf.data(), -1, utf8Buf.data(), utf8Len, nullptr, nullptr);

    return utf8Buf;
}

std::string ImGuiUtil::Utf8ToAnsi(const char* const utf8) noexcept
{
    if (utf8 == nullptr || *utf8 == '\0')
        return {};

    const int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    if (wideLen <= 0)
        return utf8;

    std::vector<wchar_t> wideBuf(wideLen);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wideBuf.data(), wideLen);

    const int ansiLen = WideCharToMultiByte(CP_ACP, 0, wideBuf.data(), -1, nullptr, 0, nullptr, nullptr);
    if (ansiLen <= 0)
        return utf8;

    std::string ansiBuf(ansiLen - 1, '\0');
    WideCharToMultiByte(CP_ACP, 0, wideBuf.data(), -1, ansiBuf.data(), ansiLen, nullptr, nullptr);

    return ansiBuf;
}

const char* ImGuiUtil::FindSubstringNoCase(const char* const haystack, const char* const needle) noexcept
{
    if (haystack == nullptr || needle == nullptr)
        return nullptr;

    const auto asciiLower = [](const char c) -> char
    {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
    };

    const std::size_t needleLen = std::strlen(needle);
    if (needleLen == 0)
        return haystack;

    const std::size_t haystackLen = std::strlen(haystack);
    if (needleLen > haystackLen)
        return nullptr;

    for (std::size_t i { 0 }; i <= haystackLen - needleLen; ++i)
    {
        std::size_t j { 0 };
        while (j < needleLen && asciiLower(haystack[i + j]) == asciiLower(needle[j]))
            ++j;
        if (j == needleLen)
            return haystack + i;
    }

    return nullptr;
}

bool ImGuiUtil::initStatus { false };
bool ImGuiUtil::win32loadStatus { false };
bool ImGuiUtil::dx9loadStatus { false };
bool ImGuiUtil::renderStatus { false };
