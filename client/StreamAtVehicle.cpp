#include "StreamAtVehicle.h"

#include <audio/bass.h>
#include <svapi.h>

#include "StreamInfo.h"

StreamAtVehicle::StreamAtVehicle(const D3DCOLOR color, std::string name,
                                 const float distance, const WORD vehicleId) noexcept
    : LocalStream(StreamType::LocalStreamAtVehicle, color, std::move(name), distance)
    , vehicleId(vehicleId)
{}

void StreamAtVehicle::Tick() noexcept
{
    this->LocalStream::Tick();

    const auto pNetGame = sv::RefNetGame();
    if (!pNetGame) return;

    const auto pVehiclePool = pNetGame->m_pPools->m_pVehicle;
    if (!pVehiclePool) return;

    const auto pVehicle = pVehiclePool->m_pObject[this->vehicleId];
    if (!pVehicle) return;

    sampapi::CMatrix matrix;
    pVehicle->GetMatrix(&matrix);

    for (const auto& channel : this->GetChannels())
    {
        if (channel->HasSpeaker())
        {
            BASS_ChannelSet3DPosition(channel->GetHandle(),
                reinterpret_cast<BASS_3DVECTOR*>(&matrix.pos),
                nullptr, nullptr);
        }
    }
}

void StreamAtVehicle::SetTarget(const BYTE /*targetType*/, const WORD targetId)
{
    this->vehicleId = targetId;
}

void StreamAtVehicle::OnChannelCreate(const Channel& channel) noexcept
{
    static const BASS_3DVECTOR kZeroVector { 0, 0, 0 };

    this->LocalStream::OnChannelCreate(channel);

    const auto pNetGame = sv::RefNetGame();
    if (!pNetGame) return;

    const auto pVehiclePool = pNetGame->m_pPools->m_pVehicle;
    if (!pVehiclePool) return;

    const auto pVehicle = pVehiclePool->m_pObject[this->vehicleId];
    if (!pVehicle) return;

    sampapi::CMatrix matrix;
    pVehicle->GetMatrix(&matrix);

    BASS_ChannelSet3DPosition(channel.GetHandle(),
        reinterpret_cast<BASS_3DVECTOR*>(&matrix.pos),
        &kZeroVector, &kZeroVector);
}
