#pragma once

#include "Core/CoreMinimal.h"

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

    struct FIntermediateMaterialLibraryData
    {
        FString                     SourcePath;
        TArray<FIntermediateMaterialData> Materials;
        TMap<FString, uint32>       NameToIndex;

        bool IsValid() const { return !Materials.empty(); }

        const FIntermediateMaterialData* FindMaterial(const FString& InName) const
        {
            auto It = NameToIndex.find(InName);
            if (It == NameToIndex.end() || It->second >= Materials.size())
            {
                return nullptr;
            }
            return &Materials[It->second];
        }
    };

} // namespace Asset
