#include "Core/CoreMinimal.h"
#include "CylinderActor.h"
#include "Engine/Component/Mesh/StaticMeshComponent.h"

ACylinderActor::ACylinderActor()
{
    SetDefaultStaticMeshPath("BasicMesh/Cylinder.obj");
    Name = "CylinderActor";
}

REGISTER_CLASS(, ACylinderActor)
