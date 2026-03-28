#include "CubeActor.h"

#include "Engine/Component/Mesh/StaticMeshComponent.h"
ACubeActor::ACubeActor()
{
    if (auto* StaticMeshComponent = GetStaticMeshComponent())
    {
        // StaticMeshComponent->SetStaticMeshPath("BasicMesh/Cube.mesh");
    }

    Name = "CubeActor";
}

REGISTER_CLASS(, ACubeActor)
