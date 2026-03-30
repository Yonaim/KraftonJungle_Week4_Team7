#include "Engine/Scene/SceneAssetBinder.h"

#include "Engine/Game/Actor.h"
#include "Engine/Component/Core/SceneComponent.h"
#include "Engine/Scene/Scene.h"

void FSceneAssetBinder::BindScene(FScene* InScene, UAssetManager* InAssetManager)
{
    if (InScene == nullptr || InAssetManager == nullptr)
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
        BindActor(Actor, InAssetManager);
    }
}

void FSceneAssetBinder::BindActor(AActor* InActor, UAssetManager* InAssetManager)
{
    if (InActor == nullptr || InAssetManager == nullptr)
    {
        return;
    }

    for (Engine::Component::USceneComponent* Component : InActor->GetOwnedComponents())
    {
        BindComponent(Component, InAssetManager);
    }
}

void FSceneAssetBinder::BindComponent(Engine::Component::USceneComponent* InComponent,
                                      UAssetManager* InAssetManager)
{
    if (InComponent == nullptr || InAssetManager == nullptr)
    {
        return;
    }

    // 현재 코드베이스에서는 각 컴포넌트가 ResolveAssetReferences()를 직접 구현하고 있으므로
    // binder는 우선 중앙 orchestration 계층으로만 동작합니다.
    //
    // 다음 단계에서 각 컴포넌트별 로직을 이 binder 내부로 옮기고,
    // SceneComponent::ResolveAssetReferences()는 제거하면 됩니다.
    InComponent->ResolveAssetReferences(InAssetManager);
}
