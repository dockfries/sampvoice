/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "PointStream.h"

#include <cassert>
#include <shared_mutex>

#include "sdk.hpp"

#include "NetHandler.h"
#include "PlayerStore.h"
#include "Header.h"

PointStream::PointStream(const float distance, const Vector3& position) : LocalStream(distance)
{
	PackWrap(this->packetStreamUpdatePosition, SV::ControlPacketType::updateLPStreamPosition, sizeof(SV::UpdateLPStreamPositionPacket));

	PackGetStruct(&*this->packetStreamUpdatePosition, SV::UpdateLPStreamPositionPacket)->stream = this->streamId;
	PackGetStruct(&*this->packetStreamUpdatePosition, SV::UpdateLPStreamPositionPacket)->position = position;
}

void PointStream::UpdatePosition(const Vector3& position)
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

	PackGetStruct(&*this->packetStreamUpdatePosition, SV::UpdateLPStreamPositionPacket)->position = position;

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
				NetHandler::SendControlPacket(listenerId, *&*this->packetStreamUpdatePosition);
		}
	}
}
