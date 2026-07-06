#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include <d3d9.h>

#include "Memory.hpp"

using CMDPROC = void(__cdecl*)(const char*);

class Samp {

    Samp() = delete;
    ~Samp() = delete;
    Samp(const Samp&) = delete;
    Samp(Samp&&) = delete;
    Samp& operator=(const Samp&) = delete;
    Samp& operator=(Samp&&) = delete;

private:

    using LoadCallback = std::function<void()>;
    using ExitCallback = std::function<void()>;

public:

    static bool Init(DWORD sampBaseAddr) noexcept;
    static bool IsInited() noexcept;
    static bool IsLoaded() noexcept;
    static void Free() noexcept;

    static void AddClientCommand(const char* cmdName, CMDPROC cmdHandler) noexcept;
    static void AddMessageToChat(D3DCOLOR color, const char* message) noexcept;
    static void ToggleSampCursor(int mode) noexcept;

public:

    static std::size_t AddLoadCallback(LoadCallback callback) noexcept;
    static std::size_t AddExitCallback(ExitCallback callback) noexcept;

    static void RemoveLoadCallback(std::size_t callback) noexcept;
    static void RemoveExitCallback(std::size_t callback) noexcept;

private:

    static void HookSampInit() noexcept;
    static void HookSampFree() noexcept;

private:

    static bool initStatus;
    static bool loadStatus;

    static std::vector<LoadCallback> loadCallbacks;
    static std::vector<ExitCallback> exitCallbacks;

    static Memory::JumpHookPtr hookSampInit;
    static Memory::JumpHookPtr hookSampFree;

};
