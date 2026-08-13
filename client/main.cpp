/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "Plugin.h"

#include <sstream>
#include <string>

static DWORD WINAPI PluginInitializationThread(const LPVOID parameter)
{
    const auto pluginModule = static_cast<HMODULE>(parameter);

    if (!Plugin::OnPluginLoad(pluginModule))
        return FALSE;

    std::string libraryName { "samp.dll" };

    if (const auto cmdLine = GetCommandLine(); cmdLine != nullptr)
    {
        std::istringstream cmdStream { cmdLine };
        std::string iString;

        while (cmdStream >> iString && iString != "-svlib");
        if (cmdStream >> iString) libraryName = std::move(iString);
    }

    HMODULE sampBaseAddress { nullptr };

    while ((sampBaseAddress = GetModuleHandle(libraryName.c_str())) == nullptr)
        SleepForMilliseconds(50);

    Plugin::OnSampLoad(sampBaseAddress);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID)
{
    if (dwReasonForCall != DLL_PROCESS_ATTACH)
        return TRUE;

    DisableThreadLibraryCalls(hModule);

    const auto initializationThread = CreateThread(NULL, 0,
        PluginInitializationThread, hModule, NULL, NULL);

    if (initializationThread == nullptr)
        return FALSE;

    CloseHandle(initializationThread);
    return TRUE;
}
