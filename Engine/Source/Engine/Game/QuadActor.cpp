#include "QuadActor.h"
#include "Engine/Component/Mesh/StaticMeshComponent.h"

#include "Engine/Component/Core/PrimitiveComponent.h"

AQuadActor::AQuadActor()
{
    SetDefaultStaticMeshPath("BasicMesh/Quad.obj");
    Name = "QuadActor";
}

REGISTER_CLASS(, AQuadActor)
