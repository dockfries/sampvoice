#include "StreamAtPlayer.h"

#include <audio/bass.h>
#include <svapi.h>

#include "StreamInfo.h"

StreamAtPlayer::StreamAtPlayer(const D3DCOLOR color, std::string name,
                               const float distance, const WORD playerId) noexcept
    : LocalStream(StreamType::LocalStreamAtPlayer, color, std::move(name), distance)
    , playerId(playerId)
{}

void StreamAtPlayer::Tick() noexcept
{
    this->LocalStream::Tick();

    const auto pNetGame = sv::RefNetGame();
    if (!pNetGame) return;

    const auto pPlayerPool = pNetGame->m_pPools->m_pPlayer;
    if (!pPlayerPool) return;

    const auto pPlayerInfo = pPlayerPool->m_pObject[this->playerId];
    if (!pPlayerInfo) return;

    const auto pPlayer = pPlayerInfo->m_pPlayer;
    if (!pPlayer) return;

    const auto pPed = pPlayer->m_pPed;
    if (!pPed) return;

    sampapi::CMatrix matrix;
    pPed->GetMatrix(&matrix);

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

void StreamAtPlayer::SetTarget(const BYTE /*targetType*/, const WORD targetId)
{
    this->playerId = targetId;
}

void StreamAtPlayer::OnChannelCreate(const Channel& channel) noexcept
{
    static const BASS_3DVECTOR kZeroVector { 0, 0, 0 };

    this->LocalStream::OnChannelCreate(channel);

    const auto pNetGame = sv::RefNetGame();
    if (!pNetGame) return;

    const auto pPlayerPool = pNetGame->m_pPools->m_pPlayer;
    if (!pPlayerPool) return;

    const auto pPlayerInfo = pPlayerPool->m_pObject[this->playerId];
    if (!pPlayerInfo) return;

    const auto pPlayer = pPlayerInfo->m_pPlayer;
    if (!pPlayer) return;

    const auto pPed = pPlayer->m_pPed;
    if (!pPed) return;

    sampapi::CMatrix matrix;
    pPed->GetMatrix(&matrix);

    BASS_ChannelSet3DPosition(channel.GetHandle(),
        reinterpret_cast<BASS_3DVECTOR*>(&matrix.pos),
        &kZeroVector, &kZeroVector);
}
