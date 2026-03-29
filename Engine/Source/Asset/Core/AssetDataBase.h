#pragma once

#include "Core/CoreMinimal.h"

namespace Asset
{
    enum class EAssetType : uint8
    {
        Unknown = 0,
        Texture,
        Material,
        StaticMesh,
        SubUVAtlas,
        FontAtlas,
    };

    struct IAssetData
    {
        virtual ~IAssetData() = default;
        virtual EAssetType GetAssetType() const = 0;
        virtual bool       IsValid() const = 0;
        virtual void       Reset() = 0;
    };
} // namespace Asset
