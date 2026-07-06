#include "Effect.h"

#include <util/Logger.h>

Effect::Effect(const DWORD type, const int priority,
               const void* const paramPtr, const DWORD paramSize)
{
    Filter filter;
    filter.type = type;
    filter.priority = priority;
    filter.params.resize(paramSize);
    if (paramSize > 0)
        std::memcpy(filter.params.data(), paramPtr, paramSize);
    this->filters.push_back(std::move(filter));
}

Effect::~Effect() noexcept
{
    for (const auto& filter : this->filters)
    {
        for (const auto& fxHandle : filter.fxHandles)
        {
            BASS_ChannelRemoveFX(fxHandle.first, fxHandle.second);
        }
    }
}

void Effect::Apply(const Channel& channel)
{
    for (auto& filter : this->filters)
    {
        if (const auto fxHandle = BASS_ChannelSetFX(channel.GetHandle(),
            filter.type, filter.priority); fxHandle != NULL)
        {
            if (BASS_FXSetParameters(fxHandle, filter.params.data()) == FALSE)
            {
                Logger::LogToFile("[sv:err:effect:apply] : failed "
                    "to set parameters (code:%d)", BASS_ErrorGetCode());
                BASS_ChannelRemoveFX(channel.GetHandle(), fxHandle);
            }
            else
            {
                filter.fxHandles[channel.GetHandle()] = fxHandle;
            }
        }
        else
        {
            Logger::LogToFile("[sv:err:effect:apply] : failed to create "
                "effect (code:%d)", BASS_ErrorGetCode());
        }
    }
}

void Effect::AppendFilter(const DWORD type, const int priority, const void* const paramPtr, const DWORD paramSize)
{
    Filter filter;
    filter.type = type;
    filter.priority = priority;
    filter.params.resize(paramSize);
    if (paramSize > 0)
        std::memcpy(filter.params.data(), paramPtr, paramSize);
    this->filters.push_back(std::move(filter));
}

void Effect::RemoveFilter(const DWORD type, const int priority)
{
    for (auto it = this->filters.begin(); it != this->filters.end(); ++it)
    {
        if (it->type == type && it->priority == priority)
        {
            for (const auto& fxHandle : it->fxHandles)
            {
                BASS_ChannelRemoveFX(fxHandle.first, fxHandle.second);
            }
            this->filters.erase(it);
            return;
        }
    }
}