#pragma once

#include <memory>
#include <vector>
#include <map>

#include <audio/bass.h>

#include "Channel.h"

struct Filter
{
    DWORD type;
    int priority;
    std::vector<BYTE> params;
    std::map<HSTREAM, HFX> fxHandles;
};

class Effect {

    Effect(const Effect&) = delete;
    Effect(Effect&&) = delete;
    Effect& operator=(const Effect&) = delete;
    Effect& operator=(Effect&&) = delete;

public:

    Effect() = default;

    explicit Effect(DWORD type, int priority,
                    const void* paramPtr, DWORD paramSize);

    ~Effect() noexcept;

public:

    void Apply(const Channel& channel);
    void AppendFilter(DWORD type, int priority, const void* paramPtr, DWORD paramSize);
    void RemoveFilter(DWORD type, int priority);

private:

    std::vector<Filter> filters;

};

using EffectPtr = std::unique_ptr<Effect>;
#define MakeEffect std::make_unique<Effect>