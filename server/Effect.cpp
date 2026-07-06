#include "EffectManager.h"
#include "StreamManager.h"
#include "Effect.h"

#include <functional>

std::atomic<uint32_t> Effect::nextEffectId{ 1 };

#include "Stream.h"
#include "NetHandler.h"
#include "Header.h"

Effect::~Effect()
{
	for (const auto stream : this->attachedStreams)
	{
		if (!StreamManager::IsValidStream(stream))
			continue;

		{
			const auto iter = this->streamPlayerCallbacks.find(stream);
			if (iter != this->streamPlayerCallbacks.end())
			{
				stream->RemovePlayerCallback(iter->second);
			}
		}

		{
			const auto iter = this->streamDeleteCallbacks.find(stream);
			if (iter != this->streamDeleteCallbacks.end())
			{
				stream->RemoveDeleteCallback(iter->second);
			}
		}

		PackGetStruct(&*this->packetDeleteEffect, SV::DeleteEffectPacket)->stream
			= stream->streamId;

		stream->SendControlPacket(*&*this->packetDeleteEffect);
	}

	EffectManager::UnregisterEffect(this);
}

void Effect::AttachStream(Stream* const stream)
{
	if (this->attachedStreams.insert(stream).second)
	{
		this->streamPlayerCallbacks[stream] =
			stream->AddPlayerCallback(std::bind(&Effect::PlayerCallback,
				this, std::placeholders::_1, std::placeholders::_2));

		this->streamDeleteCallbacks[stream] =
			stream->AddDeleteCallback(std::bind(&Effect::DeleteCallback,
				this, std::placeholders::_1));

		this->SendPacketsToPlayer(stream, SV::kNonePlayer);
	}
}

void Effect::DetachStream(Stream* const stream)
{
	if (this->attachedStreams.erase(stream))
	{
		{
			const auto iter = this->streamPlayerCallbacks.find(stream);
			if (iter != this->streamPlayerCallbacks.end())
			{
				stream->RemovePlayerCallback(iter->second);
				this->streamPlayerCallbacks.erase(iter);
			}
		}

		{
			const auto iter = this->streamDeleteCallbacks.find(stream);
			if (iter != this->streamDeleteCallbacks.end())
			{
				stream->RemoveDeleteCallback(iter->second);
				this->streamDeleteCallbacks.erase(iter);
			}
		}

		PackGetStruct(&*this->packetDeleteEffect, SV::DeleteEffectPacket)->stream
			= stream->streamId;

		stream->SendControlPacket(*&*this->packetDeleteEffect);
	}
}

bool Effect::AppendFilter(const uint32_t number, const int32_t priority, const void* const params, const uint32_t paramSize)
{
	FilterEntry entry;
	entry.number = number;
	entry.priority = priority;
	entry.params.resize(paramSize);
	if (paramSize > 0)
		std::memcpy(entry.params.data(), params, paramSize);

	this->filters_.push_back(std::move(entry));

	for (const auto stream : this->attachedStreams)
	{
		if (!StreamManager::IsValidStream(stream)) continue;

		ControlPacketContainerPtr packet;
		PackWrap(packet, SV::ControlPacketType::appendFilter, sizeof(SV::AppendFilterPacket) + paramSize);
		if (packet == nullptr) continue;

		PackGetStruct(&*packet, SV::AppendFilterPacket)->stream = stream->streamId;
		PackGetStruct(&*packet, SV::AppendFilterPacket)->effect = this->effectId;
		PackGetStruct(&*packet, SV::AppendFilterPacket)->number = number;
		PackGetStruct(&*packet, SV::AppendFilterPacket)->priority = priority;
		if (paramSize > 0)
			std::memcpy(PackGetStruct(&*packet, SV::AppendFilterPacket)->params, params, paramSize);

		stream->SendControlPacket(*&*packet);
	}

	return true;
}

bool Effect::RemoveFilter(const uint32_t number, const int32_t priority)
{
	for (auto it = this->filters_.begin(); it != this->filters_.end(); ++it)
	{
		if (it->number == number && it->priority == priority)
		{
			this->filters_.erase(it);

			for (const auto stream : this->attachedStreams)
			{
				if (!StreamManager::IsValidStream(stream)) continue;

				ControlPacketContainerPtr packet;
				PackWrap(packet, SV::ControlPacketType::removeFilter, sizeof(SV::RemoveFilterPacket));
				if (packet == nullptr) continue;

				PackGetStruct(&*packet, SV::RemoveFilterPacket)->stream = stream->streamId;
				PackGetStruct(&*packet, SV::RemoveFilterPacket)->effect = this->effectId;
				PackGetStruct(&*packet, SV::RemoveFilterPacket)->number = number;
				PackGetStruct(&*packet, SV::RemoveFilterPacket)->priority = priority;

				stream->SendControlPacket(*&*packet);
			}

			return true;
		}
	}

	return false;
}

void Effect::PlayerCallback(Stream* const stream, const uint16_t player)
{
	this->SendPacketsToPlayer(stream, player);
}

void Effect::DeleteCallback(Stream* const stream)
{
	this->attachedStreams.erase(stream);

	this->streamPlayerCallbacks.erase(stream);
	this->streamDeleteCallbacks.erase(stream);
}

void Effect::SendPacketsToPlayer(Stream* const stream, const uint16_t player)
{
	if (this->packetDeleteEffect == nullptr)
	{
		PackWrap(this->packetDeleteEffect, SV::ControlPacketType::deleteEffect, sizeof(SV::DeleteEffectPacket));
		if (this->packetDeleteEffect == nullptr) return;
		PackGetStruct(&*this->packetDeleteEffect, SV::DeleteEffectPacket)->effect = this->effectId;
	}

	for (const auto& filter : this->filters_)
	{
		ControlPacketContainerPtr packet;
		PackWrap(packet, SV::ControlPacketType::createEffect, sizeof(SV::CreateEffectPacket) + filter.params.size());
		if (packet == nullptr) continue;

		PackGetStruct(&*packet, SV::CreateEffectPacket)->stream = stream->streamId;
		PackGetStruct(&*packet, SV::CreateEffectPacket)->effect = this->effectId;
		PackGetStruct(&*packet, SV::CreateEffectPacket)->number = filter.number;
		PackGetStruct(&*packet, SV::CreateEffectPacket)->priority = filter.priority;
		if (!filter.params.empty())
			std::memcpy(PackGetStruct(&*packet, SV::CreateEffectPacket)->params, filter.params.data(), filter.params.size());

		if (player == SV::kNonePlayer)
			stream->SendControlPacket(*&*packet);
		else
			NetHandler::SendControlPacket(player, *&*packet);
	}
}