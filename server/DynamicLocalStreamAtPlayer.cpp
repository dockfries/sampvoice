/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "DynamicLocalStreamAtPlayer.h"

#include <cassert>
#include <cstring>

#include <util/memory.hpp>
#include <util/logger.h>

#include "ControlPacket.h"
#include "PlayerStore.h"
#include "Header.h"

DynamicLocalStreamAtPlayer::DynamicLocalStreamAtPlayer(
	const float distance, const uint32_t maxPlayers,
	const uint16_t playerId, const uint32_t color,
	const std::string& name
)
	: LocalStream(distance)
	, DynamicStream(distance, maxPlayers)
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

	const auto nameString = name.c_str();
	const auto nameLength = name.size() + 1;

	PackWrap(this->packetCreateStream, SV::ControlPacketType::createLStreamAtPlayer, sizeof(SV::CreateLStreamAtPacket) + nameLength);

	PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->stream = this->streamId;
	std::memcpy(PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->name, nameString, nameLength);
	PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->distance = distance;
	PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->target = playerId;
	PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->color = color;

	IPlayer* player = SampVoiceComponent::GetPlayers()->get(playerId);
	if (player != nullptr)
	{
		PlayerSortList playerList;

		const Vector3& streamPosition = player->getPosition();

		for (IPlayer* other : PlayerStore::internalPlayerPool)
		{
			Vector3 diff = other->getPosition() - streamPosition;
			float distSq = glm::dot(diff, diff);
			float maxDistSq = distance * distance;

			if (other != player && PlayerStore::IsPlayerHasPlugin(other->getID()))
			{
				if (distSq <= maxDistSq && player->getInterior() == other->getInterior() && player->getVirtualWorld() == other->getVirtualWorld())
				{
					playerList.emplace(glm::sqrt(distSq), other->getID());
				}
			}
		}

		for (const auto& playerInfo : playerList)
		{
			if (this->attachedListenersCount >= maxPlayers)
				break;

			this->Stream::AttachListener(playerInfo.playerId);
		}
	}
}

void DynamicLocalStreamAtPlayer::Tick()
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

	const auto playerId = PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->target;

	IPlayer* player = SampVoiceComponent::GetPlayers()->get(playerId);
	if (player != nullptr)
	{
		PlayerSortList playerList;

		const Vector3& streamPosition = PlayerStore::cachedPositions[playerId];
		const float streamDistance = PackGetStruct(&*this->packetStreamUpdateDistance, SV::UpdateLStreamDistancePacket)->distance;

		for (IPlayer* other : PlayerStore::internalPlayerPool)
		{
			Vector3 diff = PlayerStore::cachedPositions[other->getID()] - streamPosition;
			float distSq = glm::dot(diff, diff);
			float maxDistSq = streamDistance * streamDistance;

			if (other != player && PlayerStore::IsPlayerHasPlugin(other->getID()))
			{
				if (distSq <= maxDistSq && player->getInterior() == other->getInterior() && player->getVirtualWorld() == other->getVirtualWorld())
				{
					if (!this->HasListener(other->getID()))
					{
						playerList.emplace(glm::sqrt(distSq), other->getID());
					}
				}
				else if (this->HasListener(other->getID()))
				{
					this->Stream::DetachListener(other->getID());
				}
			}
			else if (this->HasListener(other->getID()))
			{
				this->Stream::DetachListener(other->getID());
			}
		}

		for (const auto& playerInfo : playerList)
		{
			if (this->attachedListenersCount >= this->maxPlayers)
				break;

			this->Stream::AttachListener(playerInfo.playerId);
		}
	}
}
