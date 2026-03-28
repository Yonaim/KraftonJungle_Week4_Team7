#include "Core/CoreMinimal.h"
#include "ConeActor.h"

#include "Engine/Component/Mesh/StaticMeshComponent.h"

AConeActor::AConeActor()
{
    if (auto* StaticMeshComponent = GetStaticMeshComponent())
    {
        //StaticMeshComponent->SetStaticMeshPath("BasicMesh/Cone.mesh");
    }

    Name = "ConeActor";
}

REGISTER_CLASS(, AConeActor)