/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "StreamManager.h"
#include "Stream.h"

#include <cassert>
#include <algorithm>
#include <mutex>
#include <cstring>

#include "NetHandler.h"
#include "PlayerStore.h"
#include "Header.h"

std::atomic<uint32_t> Stream::nextStreamId{ 1 };

Stream::Stream()
	: streamId(nextStreamId.fetch_add(1, std::memory_order_relaxed))
{
	PackWrap(this->packetDeleteStream, SV::ControlPacketType::deleteStream, sizeof(SV::DeleteStreamPacket));

	PackGetStruct(&*this->packetDeleteStream, SV::DeleteStreamPacket)->stream = this->streamId;

	StreamManager::RegisterStream(this);
}

Stream::~Stream() noexcept
{
	for (const auto& deleteCallback : this->deleteCallbacks)
	{
		if (deleteCallback != nullptr) deleteCallback(this);
	}

	StreamManager::UnregisterStream(this);
}

void Stream::SendVoicePacket(VoicePacket& voicePacket) const
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

	assert(voicePacket.sender >= 0 && voicePacket.sender < PLAYER_POOL_SIZE);

	if (!this->HasSpeaker(voicePacket.sender))
		return;

	voicePacket.stream = this->streamId;
	voicePacket.CalcHash();

	std::shared_lock lock(listenerMutex_);
	for (const uint16_t listenerId : listenerIds_)
	{
		if (listenerId != voicePacket.sender && PlayerStore::IsPlayerConnected(listenerId))
		{
			const auto pListenerInfo = PlayerStore::RequestPlayerWithSharedAccess(listenerId);
			const bool canReceive = pListenerInfo != nullptr &&
				pListenerInfo->listenerEnabled.load(std::memory_order_relaxed);
			PlayerStore::ReleasePlayerWithSharedAccess(listenerId);

			if (canReceive)
				NetHandler::SendVoicePacket(listenerId, voicePacket);
		}
	}
}

void Stream::SendControlPacket(ControlPacket& controlPacket) const
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

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
				NetHandler::SendControlPacket(listenerId, controlPacket);
		}
	}
}

void Stream::SetIcon(const std::string& icon)
{
	this->icon_ = icon;

	if (!this->icon_.empty())
	{
		ControlPacketContainerPtr packet;
		PackWrap(packet, SV::ControlPacketType::setStreamIcon, sizeof(SV::SetStreamIconPacket) + icon.size() + 1);
		if (packet != nullptr)
		{
			PackGetStruct(&*packet, SV::SetStreamIconPacket)->stream = this->streamId;
			std::memcpy(PackGetStruct(&*packet, SV::SetStreamIconPacket)->name, icon.c_str(), icon.size() + 1);

			this->SendControlPacket(*&*packet);
		}
	}
}

const std::string& Stream::GetIcon() const noexcept
{
	return this->icon_;
}

void Stream::SetTransiter(const bool enable) noexcept
{
	this->transiter_.store(enable, std::memory_order_relaxed);
}

bool Stream::GetTransiter() const noexcept
{
	return this->transiter_.load(std::memory_order_relaxed);
}

void Stream::UpdateTarget(const uint8_t targetType, const uint16_t targetId)
{
	ControlPacketContainerPtr packet;
	PackWrap(packet, SV::ControlPacketType::updateStreamTarget, sizeof(SV::UpdateStreamTargetPacket));
	if (packet == nullptr) return;

	PackGetStruct(&*packet, SV::UpdateStreamTargetPacket)->stream = this->streamId;
	PackGetStruct(&*packet, SV::UpdateStreamTargetPacket)->targetType = targetType;
	PackGetStruct(&*packet, SV::UpdateStreamTargetPacket)->targetId = targetId;

	this->SendControlPacket(*&*packet);
}

bool Stream::AttachListener(const uint16_t playerId)
{
	assert(playerId < PLAYER_POOL_SIZE);

	if (!PlayerStore::IsPlayerHasPlugin(playerId)) return false;
	if (this->attachedListeners[playerId].exchange(true, std::memory_order_relaxed))
		return false;

	NetHandler::SendControlPacket(playerId, *&*this->packetCreateStream);

	{
		std::unique_lock lock(listenerMutex_);
		listenerIds_.push_back(playerId);
	}

	if (!this->icon_.empty())
	{
		ControlPacketContainerPtr iconPacket;
		PackWrap(iconPacket, SV::ControlPacketType::setStreamIcon, sizeof(SV::SetStreamIconPacket) + this->icon_.size() + 1);
		if (iconPacket != nullptr)
		{
			PackGetStruct(&*iconPacket, SV::SetStreamIconPacket)->stream = this->streamId;
			std::memcpy(PackGetStruct(&*iconPacket, SV::SetStreamIconPacket)->name, this->icon_.c_str(), this->icon_.size() + 1);
			NetHandler::SendControlPacket(playerId, *&*iconPacket);
		}
	}

	for (const auto& playerCallback : this->playerCallbacks)
	{
		if (playerCallback != nullptr) playerCallback(this, playerId);
	}

	++this->attachedListenersCount;

	return true;
}

bool Stream::HasListener(const uint16_t playerId) const noexcept
{
	assert(playerId < PLAYER_POOL_SIZE);

	return this->attachedListeners[playerId].load(std::memory_order_relaxed);
}

bool Stream::DetachListener(const uint16_t playerId)
{
	assert(playerId < PLAYER_POOL_SIZE);

	if (!this->attachedListeners[playerId].exchange(false, std::memory_order_relaxed))
		return false;

	if (PlayerStore::IsPlayerConnected(playerId) && this->packetDeleteStream)
		NetHandler::SendControlPacket(playerId, *&*this->packetDeleteStream);

	{
		std::unique_lock lock(listenerMutex_);
		auto it = std::find(listenerIds_.begin(), listenerIds_.end(), playerId);
		if (it != listenerIds_.end()) listenerIds_.erase(it);
	}

	--this->attachedListenersCount;

	return true;
}

std::vector<uint16_t> Stream::DetachAllListeners()
{
	std::vector<uint16_t> detachedListeners;

	{
		std::unique_lock lock(listenerMutex_);
		detachedListeners = std::move(listenerIds_);
		listenerIds_.clear();
	}

	for (const uint16_t playerId : detachedListeners)
	{
		this->attachedListeners[playerId].store(false, std::memory_order_relaxed);
		if (PlayerStore::IsPlayerConnected(playerId) && this->packetDeleteStream)
			NetHandler::SendControlPacket(playerId, *&*this->packetDeleteStream);
	}

	this->attachedListenersCount = 0;

	return detachedListeners;
}

bool Stream::AttachSpeaker(const uint16_t playerId) noexcept
{
	assert(playerId < PLAYER_POOL_SIZE);

	if (!PlayerStore::IsPlayerHasPlugin(playerId)) return false;
	if (this->attachedSpeakers[playerId].exchange(true, std::memory_order_relaxed))
		return false;

	++this->attachedSpeakersCount;

	return true;
}

bool Stream::HasSpeaker(const uint16_t playerId) const noexcept
{
	assert(playerId < PLAYER_POOL_SIZE);

	return this->attachedSpeakers[playerId].load(std::memory_order_relaxed);
}

bool Stream::DetachSpeaker(const uint16_t playerId) noexcept
{
	assert(playerId < PLAYER_POOL_SIZE);

	if (!this->attachedSpeakers[playerId].exchange(false, std::memory_order_relaxed))
		return false;

	--this->attachedSpeakersCount;

	return true;
}

std::vector<uint16_t> Stream::DetachAllSpeakers()
{
	std::vector<uint16_t> detachedSpeakers;

	detachedSpeakers.reserve(this->attachedSpeakersCount);

	for (uint16_t iPlayerId{ 0 }; iPlayerId < PLAYER_POOL_SIZE; ++iPlayerId)
	{
		if (this->attachedSpeakers[iPlayerId].exchange(false, std::memory_order_relaxed))
			detachedSpeakers.emplace_back(iPlayerId);
	}

	this->attachedSpeakersCount = 0;

	return detachedSpeakers;
}

namespace
{
	const std::map<uint8_t, float> kDefaultValues =
	{
		{ SV::ParameterType::frequency, 0.f },
		{ SV::ParameterType::volume,    1.f },
		{ SV::ParameterType::panning,   0.f },
		{ SV::ParameterType::eaxmix,   -1.f },
		{ SV::ParameterType::src,       1.f }
	};
}

void Stream::SetParameter(const uint8_t parameter, const float value) noexcept
{
	const auto valueIter = kDefaultValues.find(parameter);
	if (valueIter == kDefaultValues.end()) return;

	const auto iter = this->parameters.try_emplace(parameter, this, parameter, value);
	if (!iter.second) iter.first->second.Set(value);
}

void Stream::ResetParameter(const uint8_t parameter) noexcept
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

	const auto valueIter = kDefaultValues.find(parameter);
	if (valueIter == kDefaultValues.end()) return;

	const auto iter = this->parameters.find(parameter);
	if (iter != this->parameters.end())
	{
		iter->second.Set(valueIter->second);

		{
			std::shared_lock lock(listenerMutex_);
			for (const uint16_t listenerId : listenerIds_)
			{
				if (PlayerStore::IsPlayerConnected(listenerId))
					iter->second.ApplyForPlayer(listenerId);
			}
		}

		this->parameters.erase(iter);
	}
}

bool Stream::HasParameter(const uint8_t parameter) const noexcept
{
	return this->parameters.find(parameter) != this->parameters.end();
}

float Stream::GetParameter(const uint8_t parameter) noexcept
{
	const auto valueIter = kDefaultValues.find(parameter);
	if (valueIter == kDefaultValues.end()) return -1.f;

	const auto iter = this->parameters.find(parameter);
	return iter != this->parameters.end() ? iter->second.Get() : valueIter->second;
}

void Stream::SlideParameterFromTo(const uint8_t parameter, const float startValue, const float endValue, const uint32_t time) noexcept
{
	const auto valueIter = kDefaultValues.find(parameter);
	if (valueIter == kDefaultValues.end()) return;

	const auto iter = this->parameters.try_emplace(parameter, this, parameter, valueIter->second);
	iter.first->second.SlideFromTo(startValue, endValue, time);
}

void Stream::SlideParameterTo(const uint8_t parameter, const float endValue, const uint32_t time) noexcept
{
	const auto valueIter = kDefaultValues.find(parameter);
	if (valueIter == kDefaultValues.end()) return;

	const auto iter = this->parameters.try_emplace(parameter, this, parameter, valueIter->second);
	iter.first->second.SlideTo(endValue, time);
}

void Stream::SlideParameter(const uint8_t parameter, const float deltaValue, const uint32_t time) noexcept
{
	const auto valueIter = kDefaultValues.find(parameter);
	if (valueIter == kDefaultValues.end()) return;

	const auto iter = this->parameters.try_emplace(parameter, this, parameter, valueIter->second);
	iter.first->second.Slide(deltaValue, time);
}

std::size_t Stream::AddPlayerCallback(PlayerCallback playerCallback) noexcept
{
	for (std::size_t i{ 0 }; i < this->playerCallbacks.size(); ++i)
	{
		if (this->playerCallbacks[i] == nullptr)
		{
			this->playerCallbacks[i] = std::move(playerCallback);
			return i;
		}
	}

	this->playerCallbacks.emplace_back(std::move(playerCallback));
	return this->playerCallbacks.size() - 1;
}

std::size_t Stream::AddDeleteCallback(DeleteCallback deleteCallback) noexcept
{
	for (std::size_t i{ 0 }; i < this->deleteCallbacks.size(); ++i)
	{
		if (this->deleteCallbacks[i] == nullptr)
		{
			this->deleteCallbacks[i] = std::move(deleteCallback);
			return i;
		}
	}

	this->deleteCallbacks.emplace_back(std::move(deleteCallback));
	return this->deleteCallbacks.size() - 1;
}

void Stream::RemovePlayerCallback(const std::size_t callback) noexcept
{
	if (callback >= this->playerCallbacks.size())
		return;

	this->playerCallbacks[callback] = nullptr;
}

void Stream::RemoveDeleteCallback(const std::size_t callback) noexcept
{
	if (callback >= this->deleteCallbacks.size())
		return;

	this->deleteCallbacks[callback] = nullptr;
}
