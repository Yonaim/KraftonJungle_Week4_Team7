#include "QuadActor.h"

#include "Engine/Component/Core/PrimitiveComponent.h"

AQuadActor::AQuadActor()
{
    if (auto* StaticMeshComponent = GetStaticMeshComponent())
    {
        // StaticMeshComponent->SetStaticMeshPath("BasicMesh/Cube.mesh");
    }
    Name = "QuadActor";
}

REGISTER_CLASS(, AQuadActor)
