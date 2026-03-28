#include "Core/CoreMinimal.h"
#include "RingActor.h"

ARingActor::ARingActor()
{
    if (auto* StaticMeshComponent = GetStaticMeshComponent())
    {
        // StaticMeshComponent->SetStaticMeshPath("BasicMesh/Ring.mesh");
    }

    Name = "RingActor";
}

REGISTER_CLASS(, ARingActor)
