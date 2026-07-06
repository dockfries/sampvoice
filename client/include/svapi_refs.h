#pragma once

// Singleton accessors for sampapi classes.
// Uses sampapi::GetAddress() for version-independent address resolution.

#include <svapi.h>

namespace sv_api {

inline sv::CChat*& RefChat() {
    static sv::CChat* p = *reinterpret_cast<sv::CChat**>(sampapi::GetAddress(0x21A0E4));
    return p;
}

inline sv::CInput*& RefInput() {
    static sv::CInput* p = *reinterpret_cast<sv::CInput**>(sampapi::GetAddress(0x21A0E8));
    return p;
}

inline sv::CScoreboard*& RefScoreboard() {
    static sv::CScoreboard* p = *reinterpret_cast<sv::CScoreboard**>(sampapi::GetAddress(0x21A0B4));
    return p;
}

inline sv::CNetGame*& RefNetGame() {
    static sv::CNetGame* p = *reinterpret_cast<sv::CNetGame**>(sampapi::GetAddress(0x21A0F8));
    return p;
}

inline sv::CGame*& RefGame() {
    static sv::CGame* p = *reinterpret_cast<sv::CGame**>(sampapi::GetAddress(0x21A10C));
    return p;
}

inline sv::CPlayerPool*& RefPlayerPool() {
    static sv::CPlayerPool* p = nullptr;
    if (auto* netgame = RefNetGame()) 
        p = netgame->m_pPools->m_pPlayer;
    return p;
}

inline sv::CVehiclePool*& RefVehiclePool() {
    static sv::CVehiclePool* p = nullptr;
    if (auto* netgame = RefNetGame())
        p = netgame->m_pPools->m_pVehicle;
    return p;
}

inline sv::CObjectPool*& RefObjectPool() {
    static sv::CObjectPool* p = nullptr;
    if (auto* netgame = RefNetGame())
        p = netgame->m_pPools->m_pObject;
    return p;
}

} // namespace sv_api
