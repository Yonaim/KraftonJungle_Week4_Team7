#include "Core/CoreMinimal.h"
#include "CylinderActor.h"

ACylinderActor::ACylinderActor()
{
    if (auto* StaticMeshComponent = GetStaticMeshComponent())
    {
        // StaticMeshComponent->SetStaticMeshPath("BasicMesh/Cylinder.mesh");
    }

    Name = "CylinderActor";
}

REGISTER_CLASS(, ACylinderActor)
