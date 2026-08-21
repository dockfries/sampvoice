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
        // wParam already carries a UTF-16 code unit from the window manager;
        // feed it straight to ImGui (unlike the previous single-byte CP_ACP
        // re-encode, this keeps multi-byte characters intact).
        if (wParam > 0 && wParam < 0x10000)
            ImGui::GetIO().AddInputCharacter(static_cast<ImWchar>(wParam));

        return TRUE;
    }

    if (uMsg == WM_IME_CHAR)
    {
        // Characters produced by an IME composition session.
        if (wParam > 0 && wParam < 0x10000)
            ImGui::GetIO().AddInputCharacter(static_cast<ImWchar>(wParam));

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
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
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

bool ImGuiUtil::initStatus { false };
bool ImGuiUtil::win32loadStatus { false };
bool ImGuiUtil::dx9loadStatus { false };
bool ImGuiUtil::renderStatus { false };
