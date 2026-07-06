/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <string>

#include <d3d9.h>

enum class StreamType
{
    None,
    GlobalStream,
    LocalStreamAtPoint,
    LocalStreamAtVehicle,
    LocalStreamAtPlayer,
    LocalStreamAtObject
};

struct StreamInfo {

    StreamInfo() noexcept = default;
    StreamInfo(const StreamInfo&) = default;
    StreamInfo(StreamInfo&&) noexcept = default;
    StreamInfo& operator=(const StreamInfo&) = default;
    StreamInfo& operator=(StreamInfo&&) noexcept = default;

public:

    StreamInfo(StreamType type, D3DCOLOR color, std::string name) noexcept;
    StreamInfo(StreamType type, D3DCOLOR color, std::string name, uint32_t streamId) noexcept;

    ~StreamInfo() noexcept = default;

    void SetIconTexture(IDirect3DTexture9* tex) noexcept { iconTexture = tex; }
    IDirect3DTexture9* GetIconTexture() const noexcept { return iconTexture; }
    uint32_t GetStreamId() const noexcept { return streamId; }

public:

    StreamType GetType() const noexcept;
    D3DCOLOR GetColor() const noexcept;
    const std::string& GetName() const noexcept;

private:

    StreamType type { StreamType::None };
    D3DCOLOR color { -1u };
    std::string name;
    uint32_t streamId{ 0 };

public:

    IDirect3DTexture9* iconTexture{ nullptr };

};
