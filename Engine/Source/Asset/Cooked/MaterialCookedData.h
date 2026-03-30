#pragma once

#include <memory>

#include "Asset/Core/MaterialTypes.h"
#include "Asset/Cooked/TextureCookedData.h"

namespace Asset
{

struct FMaterialTextureBinding
{
    EMaterialTextureSlot                Slot = EMaterialTextureSlot::Diffuse;
    std::shared_ptr<FTextureCookedData> Texture;
};

struct FMaterialCookedData
{
    FString                         SourcePath;
    FString                         Name;
    FVector                         DiffuseColor = FVector(1.0f, 1.0f, 1.0f);
    FVector                         AmbientColor = FVector(0.0f, 0.0f, 0.0f);
    FVector                         SpecularColor = FVector(0.0f, 0.0f, 0.0f);
    float                           Shininess = 0.0f;
    float                           Opacity = 1.0f;
    TArray<FMaterialTextureBinding> TextureBindings;

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

} // namespace Asset
