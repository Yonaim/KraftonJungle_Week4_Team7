#include "Core/CoreMinimal.h"
#include "StaticMeshActor.h"

#include "Engine/Component/Mesh/StaticMeshComponent.h"
#include "Engine/Component/Core/PrimitiveComponent.h"

AStaticMeshActor::AStaticMeshActor() 
{
    auto* StaticMeshComponent = new Engine::Component::UStaticMeshComponent();

}

Engine::Component::UStaticMeshComponent* AStaticMeshActor::GetStaticMeshComponent() const
{
    return Cast<Engine::Component::UStaticMeshComponent>(RootComponent);
}
