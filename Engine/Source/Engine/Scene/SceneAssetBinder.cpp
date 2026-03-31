#include "Engine/Scene/SceneAssetBinder.h"

#include "Asset/Manager/AssetCacheManager.h"
#include "Engine/Asset/FontAtlas.h"
#include "Engine/Asset/StaticMesh.h"
#include "Engine/Asset/SubUVAtlas.h"
#include "Engine/Asset/Texture.h"
#include "Engine/Asset/Material.h"
#include "Asset/Builder/MaterialBuilder.h"
#include "Engine/Component/Core/SceneComponent.h"
#include "Engine/Component/Mesh/StaticMeshComponent.h"
#include "Engine/Component/Sprite/PaperSpriteComponent.h"
#include "Engine/Component/Sprite/SubUVComponent.h"
#include "Engine/Component/Text/AtlasTextComponent.h"
#include "Engine/Game/Actor.h"
#include "Engine/Scene/Scene.h"
#include "RHI/DynamicRHI.h"

using namespace Engine::Component;

namespace
{
    template <typename TObjectType> TObjectType* EnsureAssetObject(TObjectType* ExistingObject)
    {
        return ExistingObject != nullptr ? ExistingObject : new TObjectType();
    }

    FString ResolveTexturePath(const UPaperSpriteComponent* InComponent)
    {
        if (InComponent == nullptr)
        {
            return {};
        }

        const UTexture* TextureAsset = InComponent->GetTextureAsset();
        return TextureAsset != nullptr ? TextureAsset->GetAssetPath() : InComponent->GetTexturePath();
    }

    FString ResolveSubUVPath(const USubUVComponent* InComponent)
    {
        if (InComponent == nullptr)
        {
            return {};
        }

        const USubUVAtlas* AtlasAsset = InComponent->GetSubUVAtlasAsset();
        return AtlasAsset != nullptr ? AtlasAsset->GetAssetPath() : FString();
    }

    FString ResolveFontPath(const UAtlasTextComponent* InComponent)
    {
        if (InComponent == nullptr)
        {
            return {};
        }

        const UFontAtlas* FontAsset = InComponent->GetFontAsset();
        return FontAsset != nullptr ? FontAsset->GetAssetPath() : FString();
    }

    static void BindStaticMeshMaterials(UStaticMesh*               StaticMeshAsset,
                                        Asset::FAssetCacheManager* InAssetCacheManager,
                                        RHI::FDynamicRHI*          InDynamicRHI)
    {
        if (StaticMeshAsset == nullptr || InAssetCacheManager == nullptr || InDynamicRHI == nullptr)
        {
            return;
        }

        const std::shared_ptr<Asset::FObjCookedData>& MeshCookedData =
            StaticMeshAsset->GetCookedData();
        if (MeshCookedData == nullptr)
        {
            return;
        }

        TArray<UMaterial*>& MaterialSlots = StaticMeshAsset->GetMaterialSlots();
        const size_t        SlotCount = MeshCookedData->Materials.size();

        if (MaterialSlots.size() < SlotCount)
        {
            MaterialSlots.resize(SlotCount, nullptr);
        }

        for (size_t SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
        {
            if (SlotIndex >= MeshCookedData->Materials.size())
            {
                MaterialSlots[SlotIndex] = nullptr;
                continue;
            }

            const Asset::FObjCookedMaterialRef& MaterialRef = MeshCookedData->Materials[SlotIndex];
            if (MaterialRef.Name.empty())
            {
                MaterialSlots[SlotIndex] = nullptr;
                continue;
            }

            if (MaterialRef.LibraryIndex >= MeshCookedData->MaterialLibraries.size())
            {
                MaterialSlots[SlotIndex] = nullptr;
                continue;
            }

            const FString& LibraryPath =
                MeshCookedData->MaterialLibraries[MaterialRef.LibraryIndex];
            if (LibraryPath.empty())
            {
                MaterialSlots[SlotIndex] = nullptr;
                continue;
            }

            const FString MaterialAssetPath =
                Asset::FMaterialBuilder::MakeMaterialAssetPath(LibraryPath, MaterialRef.Name);

            std::shared_ptr<Asset::FMtlCookedData> MaterialCookedData =
                InAssetCacheManager->BuildMaterial(MaterialAssetPath);

            if (MaterialCookedData == nullptr)
            {
                MaterialSlots[SlotIndex] = nullptr;
                continue;
            }

            UMaterial* MaterialAsset = EnsureAssetObject(MaterialSlots[SlotIndex]);
            if (!MaterialAsset->LoadFromCooked(MaterialAssetPath, std::move(MaterialCookedData),
                                               *InDynamicRHI))
            {
                MaterialSlots[SlotIndex] = nullptr;
                continue;
            }

            MaterialSlots[SlotIndex] = MaterialAsset;
        }
    }
} // namespace

void FSceneAssetBinder::BindScene(FScene* InScene, Asset::FAssetCacheManager* InAssetCacheManager,
                                  RHI::FDynamicRHI* InDynamicRHI)
{
    if (InScene == nullptr || InAssetCacheManager == nullptr || InDynamicRHI == nullptr)
    {
        return;
    }

    const TArray<AActor*>* Actors = InScene->GetActors();
    if (Actors == nullptr)
    {
        return;
    }

    for (AActor* Actor : *Actors)
    {
        BindActor(Actor, InAssetCacheManager, InDynamicRHI);
    }
}

void FSceneAssetBinder::BindActor(AActor* InActor, Asset::FAssetCacheManager* InAssetCacheManager,
                                  RHI::FDynamicRHI* InDynamicRHI)
{
    if (InActor == nullptr || InAssetCacheManager == nullptr || InDynamicRHI == nullptr)
    {
        return;
    }

    for (USceneComponent* Component : InActor->GetOwnedComponents())
    {
        BindComponent(Component, InAssetCacheManager, InDynamicRHI);
    }
}

void FSceneAssetBinder::BindComponent(USceneComponent*           InComponent,
                                      Asset::FAssetCacheManager* InAssetCacheManager,
                                      RHI::FDynamicRHI*          InDynamicRHI)
{
    if (InComponent == nullptr || InAssetCacheManager == nullptr || InDynamicRHI == nullptr)
    {
        return;
    }

    if (auto* StaticMeshComponent = Cast<UStaticMeshComponent>(InComponent))
    {
        const FString MeshPath = StaticMeshComponent->GetStaticMeshPath();
        if (MeshPath.empty())
        {
            StaticMeshComponent->SetStaticMeshAsset(nullptr);
            return;
        }

        std::shared_ptr<Asset::FObjCookedData> CookedData =
            InAssetCacheManager->BuildStaticMesh(MeshPath);
        if (CookedData == nullptr)
        {
            StaticMeshComponent->SetStaticMeshAsset(nullptr);
            return;
        }

        UStaticMesh* StaticMeshAsset = EnsureAssetObject(StaticMeshComponent->GetStaticMeshAsset());
        if (!StaticMeshAsset->LoadFromCooked(MeshPath, std::move(CookedData), *InDynamicRHI))
        {
            StaticMeshComponent->SetStaticMeshAsset(nullptr);
            return;
        }

        BindStaticMeshMaterials(StaticMeshAsset, InAssetCacheManager, InDynamicRHI);

        StaticMeshComponent->SetStaticMeshAsset(StaticMeshAsset);
        return;
    }

    if (auto* SubUVComponent = Cast<USubUVComponent>(InComponent))
    {
        const FString AtlasPath = ResolveSubUVPath(SubUVComponent);
        if (AtlasPath.empty())
        {
            return;
        }

        std::shared_ptr<Asset::FSubUVAtlasCookedData> CookedData =
            InAssetCacheManager->BuildSubUVAtlas(AtlasPath);
        if (CookedData == nullptr)
        {
            SubUVComponent->SetSubUVAtlasAsset(nullptr);
            return;
        }

        USubUVAtlas* AtlasAsset = EnsureAssetObject(SubUVComponent->GetSubUVAtlasAsset());
        if (!AtlasAsset->LoadFromCooked(AtlasPath, std::move(CookedData), *InDynamicRHI))
        {
            SubUVComponent->SetSubUVAtlasAsset(nullptr);
            return;
        }

        SubUVComponent->SetSubUVAtlasAsset(AtlasAsset);
        return;
    }

    if (auto* SpriteComponent = Cast<UPaperSpriteComponent>(InComponent))
    {
        const FString TexturePath = ResolveTexturePath(SpriteComponent);
        if (TexturePath.empty())
        {
            return;
        }

        std::shared_ptr<Asset::FTextureCookedData> CookedData =
            InAssetCacheManager->BuildTexture(TexturePath);
        if (CookedData == nullptr)
        {
            SpriteComponent->SetTextureAsset(nullptr);
            return;
        }

        UTexture* TextureAsset = EnsureAssetObject(SpriteComponent->GetTextureAsset());
        if (!TextureAsset->LoadFromCooked(TexturePath, std::move(CookedData), *InDynamicRHI))
        {
            SpriteComponent->SetTextureAsset(nullptr);
            return;
        }

        SpriteComponent->SetTextureAsset(TextureAsset);
        return;
    }

    if (auto* AtlasTextComponent = Cast<UAtlasTextComponent>(InComponent))
    {
        const FString FontPath = ResolveFontPath(AtlasTextComponent);
        if (FontPath.empty())
        {
            return;
        }

        std::shared_ptr<Asset::FFontAtlasCookedData> CookedData =
            InAssetCacheManager->BuildFontAtlas(FontPath);
        if (CookedData == nullptr)
        {
            AtlasTextComponent->SetFontAsset(nullptr);
            return;
        }

        UFontAtlas* FontAsset = EnsureAssetObject(AtlasTextComponent->GetFontAsset());
        if (!FontAsset->LoadFromCooked(FontPath, std::move(CookedData), *InDynamicRHI))
        {
            AtlasTextComponent->SetFontAsset(nullptr);
            return;
        }

        AtlasTextComponent->SetFontAsset(FontAsset);
        return;
    }
}
