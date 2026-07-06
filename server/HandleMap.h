#pragma once

#include <cstdint>
#include <unordered_map>

#include <pawn/amx/amx.h>

namespace HandleMap {

    inline std::unordered_map<uint32_t, void*>& GetMap() {
        static std::unordered_map<uint32_t, void*> map;
        return map;
    }

    inline uint32_t Next() {
        static uint32_t counter = 1;
        return counter++;
    }

    template<typename T>
    inline cell Store(T* ptr) {
        auto handle = Next();
        GetMap()[handle] = static_cast<void*>(ptr);
        return static_cast<cell>(handle);
    }

    template<typename T>
    inline T* Get(cell handle) {
        auto it = GetMap().find(static_cast<uint32_t>(handle));
        if (it == GetMap().end()) return nullptr;
        return static_cast<T*>(it->second);
    }

    inline void Remove(cell handle) {
        GetMap().erase(static_cast<uint32_t>(handle));
    }

    inline void Clear() {
        GetMap().clear();
    }

}
