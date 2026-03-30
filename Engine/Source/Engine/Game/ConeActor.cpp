#include "Core/CoreMinimal.h"
#include "ConeActor.h"

#include "Engine/Component/Mesh/StaticMeshComponent.h"

AConeActor::AConeActor()
{
    SetDefaultStaticMeshPath("BasicMesh/Cone.obj");
    Name = "ConeActor";
}

REGISTER_CLASS(, AConeActor)