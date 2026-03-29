#pragma once

#include "Asset/Core/Types.h"

namespace Asset
{

struct FIntermediateMaterialTextureRef
{
    FString SlotName;
    FString TexturePath;
};

struct FIntermediateMaterialData
{
    FString Name;
    FVector DiffuseColor = FVector(1.0f, 1.0f, 1.0f);
    FVector AmbientColor = FVector(0.0f, 0.0f, 0.0f);
    FVector SpecularColor = FVector(0.0f, 0.0f, 0.0f);
    float   Shininess = 0.0f;
    float   Opacity = 1.0f;

    TArray<FIntermediateMaterialTextureRef> TextureRefs;
};

} // namespace Asset
