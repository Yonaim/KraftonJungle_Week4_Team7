#pragma once

#include "Core/CoreMinimal.h"

class FScene;
class AActor;
class UAssetManager;

namespace Engine::Component
{
    class USceneComponent;
}

class ENGINE_API FSceneAssetBinder
{
  public:
    static void BindScene(FScene* InScene, UAssetManager* InAssetManager);
    static void BindActor(AActor* InActor, UAssetManager* InAssetManager);
    static void BindComponent(Engine::Component::USceneComponent* InComponent,
                              UAssetManager* InAssetManager);
};
