#pragma once

#include "Core/CoreMinimal.h"

class FScene;
class AActor;

namespace Asset
{
    class FAssetCacheManager;
}

namespace RHI
{
    class FDynamicRHI;
}

namespace Engine::Component
{
    class USceneComponent;
}

class ENGINE_API FSceneAssetBinder
{
  public:
    static void BindScene(FScene* InScene, Asset::FAssetCacheManager* InAssetCacheManager,
                          RHI::FDynamicRHI* InDynamicRHI);
    static void BindActor(AActor* InActor, Asset::FAssetCacheManager* InAssetCacheManager,
                          RHI::FDynamicRHI* InDynamicRHI);
    static void BindComponent(Engine::Component::USceneComponent* InComponent,
                              Asset::FAssetCacheManager* InAssetCacheManager,
                              RHI::FDynamicRHI* InDynamicRHI);
};
