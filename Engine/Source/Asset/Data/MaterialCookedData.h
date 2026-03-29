#pragma once

#include <memory>

#include "Asset/Data/CookedData.h"
#include "Asset/Data/TextureCookedData.h"

enum class EMaterialTextureSlot : uint8
{
    Diffuse = 0,
    Normal,
    Specular,
    Opacity,
    Emissive,
};

struct FMaterialTextureBinding
{
    EMaterialTextureSlot                Slot = EMaterialTextureSlot::Diffuse;
    std::shared_ptr<FTextureCookedData> Texture;
};

struct FMaterialCookedData : public FCookedData
{
    FString                         Name;
    FVector                         DiffuseColor = FVector(1.0f, 1.0f, 1.0f);
    FVector                         AmbientColor = FVector(0.0f, 0.0f, 0.0f);
    FVector                         SpecularColor = FVector(0.0f, 0.0f, 0.0f);
    float                           Shininess = 0.0f;
    float                           Opacity = 1.0f;
    TArray<FMaterialTextureBinding> TextureBindings;

    virtual EAssetType GetAssetType() const override { return EAssetType::Material; }

    virtual bool IsValid() const override { return !Name.empty(); }

    virtual void Reset() override
    {
        Name.clear();
        DiffuseColor = FVector(1.0f, 1.0f, 1.0f);
        AmbientColor = FVector(0.0f, 0.0f, 0.0f);
        SpecularColor = FVector(0.0f, 0.0f, 0.0f);
        Shininess = 0.0f;
        Opacity = 1.0f;
        TextureBindings.clear();
    }
};
