/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <cstddef>

/*
    A read-only view over a block of binary data. It lets the UI modules
    (Texture, BlurEffect, fonts) consume data coming either from the embedded
    resource section (Resource) or from an external file (Storage::ReadFile)
    without caring about the origin.
*/

struct ResourceData {
    const void* ptr { nullptr };
    std::size_t size { 0 };

    bool IsValid() const noexcept
    {
        return ptr != nullptr && size != 0;
    }
};
