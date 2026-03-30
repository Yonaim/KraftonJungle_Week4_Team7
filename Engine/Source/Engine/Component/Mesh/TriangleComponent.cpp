#include "TriangleComponent.h"

#include "NewRenderer/Primitive/PrimitiveTriangle.h"

namespace Engine::Component
{
    UTriangleComponent::UTriangleComponent()
    {
        static CPrimitiveTriangle trianglePrimitive;
        RenderCommand.MeshData = trianglePrimitive.GetMeshData();
    }

    REGISTER_CLASS(Engine::Component, UTriangleComponent)
} // namespace Engine::Component
