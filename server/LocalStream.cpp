/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "LocalStream.h"

#include <cassert>
#include <shared_mutex>

#include "NetHandler.h"
#include "PlayerStore.h"
#include "Header.h"

LocalStream::LocalStream(const float distance)
{
	PackWrap(this->packetStreamUpdateDistance, SV::ControlPacketType::updateLStreamDistance, sizeof(SV::UpdateLStreamDistancePacket));

	PackGetStruct(&*this->packetStreamUpdateDistance, SV::UpdateLStreamDistancePacket)->stream = this->streamId;
	PackGetStruct(&*this->packetStreamUpdateDistance, SV::UpdateLStreamDistancePacket)->distance = distance;
}

void LocalStream::UpdateDistance(const float distance)
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

	PackGetStruct(&*this->packetStreamUpdateDistance, SV::UpdateLStreamDistancePacket)->distance = distance;

	std::shared_lock lock(listenerMutex_);
	for (const uint16_t listenerId : listenerIds_)
	{
		if (PlayerStore::IsPlayerConnected(listenerId))
		{
			const auto pListenerInfo = PlayerStore::RequestPlayerWithSharedAccess(listenerId);
			const bool canReceive = pListenerInfo != nullptr &&
				pListenerInfo->listenerEnabled.load(std::memory_order_relaxed);
			PlayerStore::ReleasePlayerWithSharedAccess(listenerId);

			if (canReceive)
				NetHandler::SendControlPacket(listenerId, *&*this->packetStreamUpdateDistance);
		}
	}
}
