#pragma once

#include "Core/CoreMinimal.h"

class FScene;
class AActor;

namespace Asset
{
    class FAssetCacheManager;
}

namespace Engine::Component
{
    class USceneComponent;
    class UStaticMeshComponent;
    class USpriteComponent;
    class USubUVComponent;
    class UAtlasTextComponent;
} // namespace Engine::Component

class ENGINE_API FSceneAssetBinder
{
  public:
    static void BindScene(FScene* InScene, Asset::FAssetCacheManager* InAssetCacheManager);
    static void BindActor(AActor* InActor, Asset::FAssetCacheManager* InAssetCacheManager);
    static void BindComponent(Engine::Component::USceneComponent* InComponent,
                              Asset::FAssetCacheManager*          InAssetCacheManager);

  private:
    static void BindStaticMeshComponent(Engine::Component::UStaticMeshComponent* InComponent,
                                        Asset::FAssetCacheManager* InAssetCacheManager);
    static void BindPaperSpriteComponent(Engine::Component::UPaperSpriteComponent* InComponent,
                                         Asset::FAssetCacheManager* InAssetCacheManager);
    static void BindSubUVComponent(Engine::Component::USubUVComponent* InComponent,
                                   Asset::FAssetCacheManager*          InAssetCacheManager);
    static void BindAtlasTextComponent(Engine::Component::UAtlasTextComponent* InComponent,
                                       Asset::FAssetCacheManager*              InAssetCacheManager);
};