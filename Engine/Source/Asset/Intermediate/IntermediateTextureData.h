#pragma once

#include "Asset/Core/Types.h"

namespace Asset
{

enum class EPixelFormat : uint8
{
    Unknown = 0,
    R8,
    RGB8,
    RGBA8,
};

struct FIntermediateTextureData
{
    uint32        Width = 0;
    uint32        Height = 0;
    uint32        Channels = 0;
    EPixelFormat  Format = EPixelFormat::Unknown;
    TArray<uint8> Pixels;
};

} // namespace Asset
