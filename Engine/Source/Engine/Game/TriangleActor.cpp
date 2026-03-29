#include "TriangleActor.h"

#include "Engine/Component/Core/PrimitiveComponent.h"

ATriangleActor::ATriangleActor()
{
    if (auto* StaticMeshComponent = GetStaticMeshComponent())
    {
        // StaticMeshComponent->SetStaticMeshPath("BasicMesh/Cube.mesh");
    }
    Name = "TriangleActor";
}

REGISTER_CLASS(, ATriangleActor)
