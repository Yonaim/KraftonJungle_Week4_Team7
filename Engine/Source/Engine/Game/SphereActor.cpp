#include "Core/CoreMinimal.h"
#include "SphereActor.h"

ASphereActor::ASphereActor()
{
    if (auto* StaticMeshComponent = GetStaticMeshComponent())
    {
        // StaticMeshComponent->SetStaticMeshPath("BasicMesh/Sphere.mesh");
    }

    Name = "SphereActor";
}
REGISTER_CLASS(, ASphereActor)