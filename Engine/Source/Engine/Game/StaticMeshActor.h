#pragma once

#include "Actor.h"

namespace Engine::Component
{
    class UStaticMeshComponent;
} // namespace Engine::Component


class ENGINE_API AStaticMeshActor:public AActor
{
    DECLARE_RTTI(AStaticMeshActor, AActor)
  public:
    AStaticMeshActor();
    ~AStaticMeshActor() override = default;

    Engine::Component::UStaticMeshComponent* GetStaticMeshComponent() const;

};
