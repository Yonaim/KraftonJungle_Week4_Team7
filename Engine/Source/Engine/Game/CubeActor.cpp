#include "CubeActor.h"

#include "Engine/Component/Mesh/StaticMeshComponent.h"
ACubeActor::ACubeActor()
{
    SetDefaultStaticMeshPath("BasicMesh/Cube.obj");
    Name = "CubeActor";
}

REGISTER_CLASS(, ACubeActor)
