#pragma once

#include "Asset/Core/MaterialTypes.h"

namespace Asset
{

    struct FMtlTextureBinding
    {
        EMaterialTextureSlot Slot = EMaterialTextureSlot::Diffuse;
        FString              TexturePath;
    };

    struct FMtlCookedData
    {
        FString                    SourcePath;
        FString                    Name;
        FVector                    DiffuseColor = FVector(1.0f, 1.0f, 1.0f);
        FVector                    AmbientColor = FVector(0.0f, 0.0f, 0.0f);
        FVector                    SpecularColor = FVector(0.0f, 0.0f, 0.0f);
        float                      Shininess = 0.0f;
        float                      Opacity = 1.0f;
        TArray<FMtlTextureBinding> TextureBindings;

        bool IsValid() const { return !Name.empty(); }

        void Reset()
        {
            SourcePath.clear();
            Name.clear();
            DiffuseColor = FVector(1.0f, 1.0f, 1.0f);
            AmbientColor = FVector(0.0f, 0.0f, 0.0f);
            SpecularColor = FVector(0.0f, 0.0f, 0.0f);
            Shininess = 0.0f;
            Opacity = 1.0f;
            TextureBindings.clear();
        }
    };

    struct FMtlCookedLibraryData
    {
        FString                SourcePath;
        TArray<FMtlCookedData> Materials;
        TMap<FString, uint32>  NameToIndex;

        bool IsValid() const { return !Materials.empty(); }

        const FMtlCookedData* FindMaterial(const FString& InName) const
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
