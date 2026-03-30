#include "Engine/Scene/SceneAssetBinder.h"

#include "Asset/Manager/AssetCacheManager.h"

#include "Engine/Game/Actor.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Component/Core/SceneComponent.h"
#include "Engine/Component/Mesh/StaticMeshComponent.h"
#include "Engine/Component/Sprite/PaperSpriteComponent.h"
#include "Engine/Component/Sprite/SubUVComponent.h"
#include "Engine/Component/Text/AtlasTextComponent.h"

#include "Asset/StaticMesh.h"
#include "Asset/Texture.h"
#include "Asset/SubUVAtlas.h"
#include "Asset/FontAtlas.h"

using namespace Engine::Component;

void FSceneAssetBinder::BindScene(FScene* InScene, Asset::FAssetCacheManager* InAssetCacheManager)
{
    if (InScene == nullptr || InAssetCacheManager == nullptr)
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
        BindActor(Actor, InAssetCacheManager);
    }
}

void FSceneAssetBinder::BindActor(AActor* InActor, Asset::FAssetCacheManager* InAssetCacheManager)
{
    if (InActor == nullptr || InAssetCacheManager == nullptr)
    {
        return;
    }

    for (USceneComponent* Component : InActor->GetOwnedComponents())
    {
        BindComponent(Component, InAssetCacheManager);
    }
}

void FSceneAssetBinder::BindComponent(USceneComponent*           InComponent,
                                      Asset::FAssetCacheManager* InAssetCacheManager)
{
    if (InComponent == nullptr || InAssetCacheManager == nullptr)
    {
        return;
    }

    if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(InComponent))
    {
        BindStaticMeshComponent(StaticMeshComponent, InAssetCacheManager);
        return;
    }

    if (USubUVComponent* SubUVComponent = Cast<USubUVComponent>(InComponent))
    {
        BindSubUVComponent(SubUVComponent, InAssetCacheManager);
        return;
    }

    if (UPaperSpriteComponent* PaperSpriteComponent = Cast<UPaperSpriteComponent>(InComponent))
    {
        BindPaperSpriteComponent(PaperSpriteComponent, InAssetCacheManager);
        return;
    }

    if (UAtlasTextComponent* AtlasTextComponent = Cast<UAtlasTextComponent>(InComponent))
    {
        BindAtlasTextComponent(AtlasTextComponent, InAssetCacheManager);
        return;
    }
}

void FSceneAssetBinder::BindStaticMeshComponent(UStaticMeshComponent*      InComponent,
                                                Asset::FAssetCacheManager* InAssetCacheManager)
{
    if (InComponent == nullptr || InAssetCacheManager == nullptr)
    {
        return;
    }

    const FString MeshPath = InComponent->GetStaticMeshPath();
    if (MeshPath.empty())
    {
        InComponent->SetStaticMeshAsset(nullptr);
        return;
    }

    Asset::FStaticMeshCookedData* CookedMesh = InAssetCacheManager->BuildStaticMesh(MeshPath);
    if (CookedMesh == nullptr)
    {
        InComponent->SetStaticMeshAsset(nullptr);
        return;
    }

    UStaticMesh* StaticMeshAsset = InComponent->GetStaticMeshAsset();
    if (StaticMeshAsset == nullptr)
    {
        StaticMeshAsset = new UStaticMesh();
    }

    StaticMeshAsset->SetCookedData(CookedMesh);
    StaticMeshAsset->SetAssetPath(MeshPath);

    InComponent->SetStaticMeshAsset(StaticMeshAsset);
}

void FSceneAssetBinder::BindPaperSpriteComponent(UPaperSpriteComponent*          InComponent,
                                            Asset::FAssetCacheManager* InAssetCacheManager)
{
    if (InComponent == nullptr || InAssetCacheManager == nullptr)
    {
        return;
    }

    const FString TexturePath = InComponent->GetTexturePath();
    if (TexturePath.empty())
    {
        InComponent->SetTextureAsset(nullptr);
        return;
    }

    Asset::FTextureCookedData* CookedTexture = InAssetCacheManager->BuildTexture(TexturePath);
    if (CookedTexture == nullptr)
    {
        InComponent->SetTextureAsset(nullptr);
        return;
    }

    UTexture* TextureAsset = InComponent->GetTextureAsset();
    if (TextureAsset == nullptr)
    {
        TextureAsset = new UTexture();
    }

    TextureAsset->SetCookedData(CookedTexture);
    TextureAsset->SetAssetPath(TexturePath);

    InComponent->SetTextureAsset(TextureAsset);
}

void FSceneAssetBinder::BindSubUVComponent(USubUVComponent*           InComponent,
                                           Asset::FAssetCacheManager* InAssetCacheManager)
{
    if (InComponent == nullptr || InAssetCacheManager == nullptr)
    {
        return;
    }

    const FString AtlasPath = InComponent->GetSubUVAtlasPath();
    if (AtlasPath.empty())
    {
        InComponent->SetSubUVAtlasAsset(nullptr);
        return;
    }

    Asset::FSubUVAtlasCookedData* CookedAtlas = InAssetCacheManager->BuildSubUVAtlas(AtlasPath);
    if (CookedAtlas == nullptr)
    {
        InComponent->SetSubUVAtlasAsset(nullptr);
        return;
    }

    USubUVAtlas* AtlasAsset = InComponent->GetSubUVAtlasAsset();
    if (AtlasAsset == nullptr)
    {
        AtlasAsset = new USubUVAtlas();
    }

    AtlasAsset->SetCookedData(CookedAtlas);
    AtlasAsset->SetAssetPath(AtlasPath);

    InComponent->SetSubUVAtlasAsset(AtlasAsset);
}

void FSceneAssetBinder::BindAtlasTextComponent(UAtlasTextComponent*       InComponent,
                                               Asset::FAssetCacheManager* InAssetCacheManager)
{
    if (InComponent == nullptr || InAssetCacheManager == nullptr)
    {
        return;
    }

    const FString FontPath = InComponent->GetFontAtlasPath();
    if (FontPath.empty())
    {
        InComponent->SetFontAtlasAsset(nullptr);
        return;
    }

    Asset::FFontAtlasCookedData* CookedFontAtlas = InAssetCacheManager->BuildFontAtlas(FontPath);
    if (CookedFontAtlas == nullptr)
    {
        InComponent->SetFontAtlasAsset(nullptr);
        return;
    }

    UFontAtlas* FontAtlasAsset = InComponent->GetFontAtlasAsset();
    if (FontAtlasAsset == nullptr)
    {
        FontAtlasAsset = new UFontAtlas();
    }

    FontAtlasAsset->SetCookedData(CookedFontAtlas);
    FontAtlasAsset->SetAssetPath(FontPath);

    InComponent->SetFontAtlasAsset(FontAtlasAsset);
}