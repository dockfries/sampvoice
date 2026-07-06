#include "StreamAtObject.h"

#include <audio/bass.h>
#include <svapi.h>

#include "StreamInfo.h"

StreamAtObject::StreamAtObject(const D3DCOLOR color, std::string name,
                               const float distance, const WORD objectId) noexcept
    : LocalStream(StreamType::LocalStreamAtObject, color, std::move(name), distance)
    , objectId(objectId)
{}

void StreamAtObject::Tick() noexcept
{
    this->LocalStream::Tick();

    const auto pNetGame = sv::RefNetGame();
    if (!pNetGame) return;

    const auto pObjectPool = pNetGame->m_pPools->m_pObject;
    if (!pObjectPool) return;

    const auto pObject = pObjectPool->m_pObject[this->objectId];
    if (!pObject) return;

    sampapi::CMatrix matrix;
    pObject->GetMatrix(&matrix);

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

void StreamAtObject::SetTarget(const BYTE /*targetType*/, const WORD targetId)
{
    this->objectId = targetId;
}

void StreamAtObject::OnChannelCreate(const Channel& channel) noexcept
{
    static const BASS_3DVECTOR kZeroVector { 0, 0, 0 };

    this->LocalStream::OnChannelCreate(channel);

    const auto pNetGame = sv::RefNetGame();
    if (!pNetGame) return;

    const auto pObjectPool = pNetGame->m_pPools->m_pObject;
    if (!pObjectPool) return;

    const auto pObject = pObjectPool->m_pObject[this->objectId];
    if (!pObject) return;

    sampapi::CMatrix matrix;
    pObject->GetMatrix(&matrix);

    BASS_ChannelSet3DPosition(channel.GetHandle(),
        reinterpret_cast<BASS_3DVECTOR*>(&matrix.pos),
        &kZeroVector, &kZeroVector);
}
