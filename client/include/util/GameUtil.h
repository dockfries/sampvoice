#pragma once

#include <Windows.h>

#include <svapi.h>
#include <game/CRect.h>

class GameUtil {

    GameUtil() = delete;
    ~GameUtil() = delete;
    GameUtil(const GameUtil&) = delete;
    GameUtil(GameUtil&&) = delete;
    GameUtil& operator=(const GameUtil&) = delete;
    GameUtil& operator=(GameUtil&&) = delete;

public:

    static bool IsKeyPressed(int keyId) noexcept;
    static bool IsMenuActive() noexcept;
    static bool IsWindowActive() noexcept;
    static bool IsGameActive() noexcept;
    static bool HasPlayerPed() noexcept;
    static bool IsPlayerVisible(WORD playerId) noexcept;
    static bool GetRadarRect(CRect& radarRect) noexcept;
    static void DisableAntiCheat();

};
