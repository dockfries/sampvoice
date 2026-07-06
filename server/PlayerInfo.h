#pragma once

#include <atomic>
#include <cstdint>
#include <map>
#include <shared_mutex>

#include "Stream.h"

struct PlayerInfo {

	PlayerInfo() = delete;
	PlayerInfo(const PlayerInfo&) = delete;
	PlayerInfo(PlayerInfo&&) = delete;
	PlayerInfo& operator=(const PlayerInfo&) = delete;
	PlayerInfo& operator=(PlayerInfo&&) = delete;

	static constexpr uint32_t kAllChannels = 0xFFFFFFFF;

public:

	PlayerInfo(uint8_t pluginVersion, bool microStatus) noexcept
		: pluginVersion(pluginVersion), microStatus(microStatus) {}

	~PlayerInfo() noexcept = default;

public:

	const uint8_t pluginVersion{ NULL };
	const bool microStatus{ false };

	std::atomic_bool muteStatus{ false };
	std::atomic_bool recordStatus{ false };
	std::atomic_bool speakerEnabled{ true };
	std::atomic_bool listenerEnabled{ true };
	std::atomic<uint32_t> activeChannels{ 0 };
	std::atomic<uint32_t> enabledChannels{ kAllChannels };
	mutable std::shared_mutex streamsMutex;
	std::set<Stream*> listenerStreams;
	std::map<Stream*, uint32_t> speakerStreams;
	std::map<uint8_t, uint32_t> keys;

};