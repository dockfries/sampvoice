/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "BlackList.h"

#include <fstream>

#include <svapi.h>
#include <util/Addresses.h>
#include <util/Logger.h>

BlackList::LockedPlayer::LockedPlayer(std::string playerName, const WORD playerId) noexcept
    : playerName(std::move(playerName)), playerId(playerId) {}

bool BlackList::Load(const std::string& filePath)
{
    constexpr auto kFileMode = std::ios::in;

    std::ifstream blackListFile { filePath, kFileMode };

    if (!blackListFile || !blackListFile.is_open())
        return false;

    BlackList::blackList.clear();

    {
        std::string nickName;

        while (std::getline(blackListFile, nickName) && !nickName.empty())
        {
            WORD playerId { SV::kNonePlayer };

            if (const auto pNetGame = sv::RefNetGame(); pNetGame != nullptr)
            {
                if (const auto pPlayerPool = pNetGame->m_pPools->m_pPlayer; pPlayerPool != nullptr)
                {
                    for (WORD iPlayerId { 0 }; iPlayerId < sv::CPlayerPool::MAX_PLAYERS; ++iPlayerId)
                    {
                        if (pPlayerPool->m_pObject[iPlayerId] == nullptr) continue;
                        if (const auto playerName = pPlayerPool->m_pObject[iPlayerId]->m_szNick.c_str(); playerName != nullptr)
                        {
                            if (nickName == playerName)
                            {
                                playerId = iPlayerId;
                                break;
                            }
                        }
                    }
                }
            }

            BlackList::blackList.emplace_back(std::move(nickName), playerId);
        }
    }

    return true;
}

bool BlackList::Save(const std::string& filePath)
{
    constexpr auto kFileMode = std::ios::out | std::ios::trunc;

    std::ofstream blackListFile { filePath, kFileMode };

    if (!blackListFile || !blackListFile.is_open())
        return false;

    for (const auto& playerInfo : BlackList::blackList)
    {
        blackListFile << playerInfo.playerName << std::endl;
    }

    return true;
}

bool BlackList::Init() noexcept
{
    if (BlackList::initStatus)
        return false;

    Logger::LogToFile("[sv:dbg:blacklist:init] : module initializing...");

    try
    {
        BlackList::deletePlayerInPoolHook = MakeJumpHook(Addresses::GetDeletePlayerFromPoolFunc(),
                                                          BlackList::DeletePlayerFromPoolHook);
        BlackList::createPlayerInPoolHook = MakeJumpHook(Addresses::GetCreatePlayerInPoolFunc(),
                                                          BlackList::CreatePlayerInPoolHook);
    }
    catch (const std::exception& exception)
    {
        Logger::LogToFile("[sv:err:blacklist:init] : failed to create function hooks");
        BlackList::createPlayerInPoolHook.reset();
        BlackList::deletePlayerInPoolHook.reset();
        return false;
    }

    Logger::LogToFile("[sv:dbg:blacklist:init] : module initialized");

    BlackList::initStatus = true;

    return true;
}

void BlackList::Free() noexcept
{
    if (!BlackList::initStatus)
        return;

    Logger::LogToFile("[sv:dbg:blacklist:free] : module releasing...");

    BlackList::createPlayerInPoolHook.reset();
    BlackList::deletePlayerInPoolHook.reset();

    BlackList::blackList.clear();

    Logger::LogToFile("[sv:dbg:blacklist:free] : module released");

    BlackList::initStatus = false;
}

void BlackList::LockPlayer(const WORD playerId)
{
    if (playerId != SV::kNonePlayer)
    {
        if (const auto pNetGame = sv::RefNetGame();
            pNetGame != nullptr)
        {
            if (const auto pPlayerPool = pNetGame->m_pPools->m_pPlayer;
                pPlayerPool != nullptr)
            {
                if (pPlayerPool->m_pObject[playerId] == nullptr) return;
                if (const auto playerName = pPlayerPool->m_pObject[playerId]->m_szNick.c_str();
                    playerName != nullptr)
                {
                    for (const auto& playerInfo : BlackList::blackList)
                    {
                        if (playerInfo.playerName == playerName)
                            return;
                    }

                    Logger::LogToFile("[sv:dbg:blacklist:lockplayer] : adding player "
                        "(id:%hu;nick:%s) to blacklist...", playerId, playerName);

                    BlackList::blackList.emplace_front(playerName, playerId);
                }
            }
        }
    }
}

void BlackList::UnlockPlayer(const WORD playerId)
{
    Logger::LogToFile("[sv:dbg:blacklist:unlockplayer] : removing "
        "player (%hu) from blacklist...", playerId);

    BlackList::blackList.remove_if([&playerId](const auto& object)
    {
        return playerId == object.playerId;
    });
}

void BlackList::UnlockPlayer(const std::string& playerName)
{
    Logger::LogToFile("[sv:dbg:blacklist:unlockplayer] : removing "
        "player (%s) from blacklist...", playerName.c_str());

    BlackList::blackList.remove_if([&playerName](const auto& object)
    {
        return playerName == object.playerName;
    });
}

const std::list<BlackList::LockedPlayer>& BlackList::RequestBlackList() noexcept
{
    return BlackList::blackList;
}

bool BlackList::IsPlayerBlocked(const WORD playerId) noexcept
{
    if (playerId != SV::kNonePlayer)
    {
        for (const auto& playerInfo : BlackList::blackList)
        {
            if (playerInfo.playerId == playerId)
                return true;
        }
    }

    return false;
}

bool BlackList::IsPlayerBlocked(const std::string& playerName) noexcept
{
    for (const auto& playerInfo : BlackList::blackList)
    {
        if (playerInfo.playerName == playerName)
            return true;
    }

    return false;
}

BOOL __thiscall BlackList::CreatePlayerInPoolHook(sv::CPlayerPool* const _this,
    const sampapi::ID nId, const char* const szName, const BOOL bIsNPC) noexcept
{
    BlackList::createPlayerInPoolHook->Disable();
    const auto retStatus = static_cast<BOOL(__thiscall*)(sv::CPlayerPool*, sampapi::ID, const char*, BOOL)>
        (BlackList::createPlayerInPoolHook->GetPatch().memAddr)(_this, nId, szName, bIsNPC);
    BlackList::createPlayerInPoolHook->Enable();

    if (retStatus == TRUE && bIsNPC == FALSE)
    {
        for (auto& playerInfo : BlackList::blackList)
        {
            if (playerInfo.playerName == szName)
            {
                playerInfo.playerId = nId;
                break;
            }
        }
    }

    return retStatus;
}

BOOL __thiscall BlackList::DeletePlayerFromPoolHook(sv::CPlayerPool* const _this,
    const sampapi::ID nId, const int nReason) noexcept
{
    for (auto& playerInfo : BlackList::blackList)
    {
        if (playerInfo.playerId == nId)
        {
            playerInfo.playerId = SV::kNonePlayer;
            break;
        }
    }

    BlackList::deletePlayerInPoolHook->Disable();
    const auto retStatus = static_cast<BOOL(__thiscall*)(sv::CPlayerPool*, sampapi::ID, int)>
        (BlackList::deletePlayerInPoolHook->GetPatch().memAddr)(_this, nId, nReason);
    BlackList::deletePlayerInPoolHook->Enable();

    return retStatus;
}

bool BlackList::initStatus { false };

std::list<BlackList::LockedPlayer> BlackList::blackList;

Memory::JumpHookPtr BlackList::createPlayerInPoolHook { nullptr };
Memory::JumpHookPtr BlackList::deletePlayerInPoolHook { nullptr };
