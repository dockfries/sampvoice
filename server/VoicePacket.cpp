/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "VoicePacket.h"

#if defined(__SSE4_2__)
#include <smmintrin.h>
#elif defined(_MSC_VER)
#include <intrin.h>
#endif

static uint32_t CalcCrc32cHash(const uint8_t* buffer, uint32_t length, uint32_t crc = 0) noexcept
{
	crc = ~crc;

#if defined(__SSE4_2__) || defined(_MSC_VER)
	while (length >= 4)
	{
		crc = static_cast<uint32_t>(_mm_crc32_u32(crc, *reinterpret_cast<const uint32_t*>(buffer)));
		buffer += 4;
		length -= 4;
	}
	if (length & 2)
	{
		crc = static_cast<uint32_t>(_mm_crc32_u16(crc, *reinterpret_cast<const uint16_t*>(buffer)));
		buffer += 2;
	}
	if (length & 1)
	{
		crc = static_cast<uint32_t>(_mm_crc32_u8(crc, *buffer));
	}
#else
	while (length--)
	{
		crc ^= *buffer++;

		for (int k = 0; k < 8; ++k) crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
	}
#endif

	return ~crc;
}

uint32_t VoicePacket::GetFullSize() const noexcept
{
	return sizeof(*this) + this->length;
}

bool VoicePacket::CheckHeader() const noexcept
{
	return this->hash == CalcCrc32cHash(
		(uint8_t*)(this) + sizeof(this->hash),
		sizeof(*this) - sizeof(this->hash)
	);
}

void VoicePacket::CalcHash() noexcept
{
	this->hash = CalcCrc32cHash(
		(uint8_t*)(this) + sizeof(this->hash),
		sizeof(*this) - sizeof(this->hash)
	);
}
