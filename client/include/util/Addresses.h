#pragma once

#include <cstdint>
#include <svapi.h>

namespace Addresses {

inline uintptr_t GetRcInitAddr() {
#if defined(SAMP_R1)
    return sampapi::GetBase() + 0x2401D3;
#elif defined(SAMP_R3)
    return sampapi::GetBase() + 0xB658;
#elif defined(SAMP_R5)
    return sampapi::GetBase() + 0xB998;
#elif defined(SAMP_DL)
    return sampapi::GetBase() + 0xB678;
#else
#error Unknown SAMP version
#endif
}

inline uintptr_t GetBassInitCallAddr() {
#if defined(SAMP_R1)
    return sampapi::GetBase() + 0x628DF;
#elif defined(SAMP_R3)
    return sampapi::GetBase() + 0x65D2F;
#elif defined(SAMP_R5)
    return sampapi::GetBase() + 0x6649F;
#elif defined(SAMP_DL)
    return sampapi::GetBase() + 0x65F1F;
#else
#error Unknown SAMP version
#endif
}

inline uintptr_t GetBassSetConfigAddr() {
#if defined(SAMP_R1)
    return sampapi::GetBase() + 0x6290F;
#elif defined(SAMP_R3)
    return sampapi::GetBase() + 0x65D5F;
#elif defined(SAMP_R5)
    return sampapi::GetBase() + 0x664CF;
#elif defined(SAMP_DL)
    return sampapi::GetBase() + 0x65F4F;
#else
#error Unknown SAMP version
#endif
}

inline uintptr_t GetSampInitAddr() {
#if defined(SAMP_R1)
    return sampapi::GetBase() + 0x2565E2;
#elif defined(SAMP_R3)
    return sampapi::GetBase() + 0xC57E2;
#elif defined(SAMP_R5)
    return sampapi::GetBase() + 0xC4F52;
#elif defined(SAMP_DL)
    return sampapi::GetBase() + 0xC6614;
#else
#error Unknown SAMP version
#endif
}

inline uintptr_t GetSampDestructAddr() {
#if defined(SAMP_R1)
    return sampapi::GetBase() + 0x9380;
#elif defined(SAMP_R3)
    return sampapi::GetBase() + 0x9510;
#elif defined(SAMP_R5)
    return sampapi::GetBase() + 0x9880;
#elif defined(SAMP_DL)
    return sampapi::GetBase() + 0x9570;
#else
#error Unknown SAMP version
#endif
}

inline uintptr_t GetOpenChatFunc() {
#if defined(SAMP_R1)
    return sampapi::GetBase() + 0x657E0;
#elif defined(SAMP_R3)
    return sampapi::GetBase() + 0x68D10;
#elif defined(SAMP_R5)
    return sampapi::GetBase() + 0x69480;
#elif defined(SAMP_DL)
    return sampapi::GetBase() + 0x68EC0;
#else
#error Unknown SAMP version
#endif
}

inline uintptr_t GetSwitchModeFunc() {
#if defined(SAMP_R1)
    return sampapi::GetBase() + 0x5D7B0;
#elif defined(SAMP_R3)
    return sampapi::GetBase() + 0x60B50;
#elif defined(SAMP_R5)
    return sampapi::GetBase() + 0x612C0;
#elif defined(SAMP_DL)
    return sampapi::GetBase() + 0x60D40;
#else
#error Unknown SAMP version
#endif
}

inline uintptr_t GetOpenScoreboardFunc() {
#if defined(SAMP_R1)
    return sampapi::GetBase() + 0x6AD30;
#elif defined(SAMP_R3)
    return sampapi::GetBase() + 0x6EC80;
#elif defined(SAMP_R5)
    return sampapi::GetBase() + 0x6F3D0;
#elif defined(SAMP_DL)
    return sampapi::GetBase() + 0x6EE10;
#else
#error Unknown SAMP version
#endif
}

inline uintptr_t GetCreatePlayerInPoolFunc() {
#if defined(SAMP_R1)
    return sampapi::GetBase() + 0x10D50;
#elif defined(SAMP_R3)
    return sampapi::GetBase() + 0x13E80;
#elif defined(SAMP_R5)
    return sampapi::GetBase() + 0x14250;
#elif defined(SAMP_DL)
    return sampapi::GetBase() + 0x14100;
#else
#error Unknown SAMP version
#endif
}

inline uintptr_t GetDeletePlayerFromPoolFunc() {
#if defined(SAMP_R1)
    return sampapi::GetBase() + 0x10B90;
#elif defined(SAMP_R3)
    return sampapi::GetBase() + 0x13CB0;
#elif defined(SAMP_R5)
    return sampapi::GetBase() + 0x14090;
#elif defined(SAMP_DL)
    return sampapi::GetBase() + 0x13F40;
#else
#error Unknown SAMP version
#endif
}

}
