#pragma once

#include <cstdint>
#include <unordered_set>
#include <unordered_map>
#include <atomic>
#include <vector>

#include "ControlPacket.h"
#include "Header.h"

struct ChorusParameters
{
	float wetdrymix;
	float depth;
	float feedback;
	float frequency;
	uint32_t waveform;
	float delay;
	uint32_t phase;
};

struct CompressorParameters
{
	float gain;
	float attack;
	float release;
	float threshold;
	float ratio;
	float predelay;
};

struct DistortionParameters
{
	float gain;
	float edge;
	float posteqcenterfrequency;
	float posteqbandwidth;
	float prelowpasscutoff;
};

struct EchoParameters
{
	float wetdrymix;
	float feedback;
	float leftdelay;
	float rightdelay;
	bool pandelay;
};

struct FlangerParameters
{
	float wetdrymix;
	float depth;
	float feedback;
	float frequency;
	uint32_t waveform;
	float delay;
	uint32_t phase;
};

struct GargleParameters
{
	uint32_t ratehz;
	uint32_t waveshape;
};

struct I3dl2reverbParameters
{
	int room;
	int roomhf;
	float roomrollofffactor;
	float decaytime;
	float decayhfratio;
	int reflections;
	float reflectionsdelay;
	int reverb;
	float reverbdelay;
	float diffusion;
	float density;
	float hfreference;
};

struct ParameqParameters
{
	float center;
	float bandwidth;
	float gain;
};

struct ReverbParameters
{
	float ingain;
	float reverbmix;
	float reverbtime;
	float highfreqrtratio;
};

struct FilterEntry
{
	uint32_t number;
	int32_t priority;
	std::vector<uint8_t> params;
};

class Effect {

	Effect(const Effect&) = delete;
	Effect(Effect&&) = delete;
	Effect& operator=(const Effect&) = delete;
	Effect& operator=(Effect&&) = delete;

	static std::atomic<uint32_t> nextEffectId;

	const uint32_t effectId;

public:

	Effect()
		: effectId(nextEffectId.fetch_add(1, std::memory_order_relaxed))
	{
	}

	template<class ParametersType>
	explicit Effect(const uint32_t number, const int priority, const ParametersType& parameters)
		: effectId(nextEffectId.fetch_add(1, std::memory_order_relaxed))
	{
		FilterEntry entry;
		entry.number = number;
		entry.priority = priority;
		entry.params.resize(sizeof(parameters));
		std::memcpy(entry.params.data(), &parameters, sizeof(parameters));
		filters_.push_back(std::move(entry));
	}

	virtual ~Effect();

public:

	void AttachStream(class Stream* stream);
	void DetachStream(class Stream* stream);

	bool AppendFilter(uint32_t number, int32_t priority, const void* params, uint32_t paramSize);
	bool RemoveFilter(uint32_t number, int32_t priority);

private:

	void PlayerCallback(class Stream* stream, uint16_t player);
	void DeleteCallback(class Stream* stream);
	void SendPacketsToPlayer(class Stream* stream, uint16_t player);

private:

	std::vector<FilterEntry> filters_;

	std::unordered_set<class Stream*> attachedStreams;

	std::unordered_map<class Stream*, std::size_t> streamPlayerCallbacks;
	std::unordered_map<class Stream*, std::size_t> streamDeleteCallbacks;

	ControlPacketContainerPtr packetDeleteEffect{ nullptr };

};