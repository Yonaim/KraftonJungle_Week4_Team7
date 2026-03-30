#include "QuadComponent.h"

#include "NewRenderer/Primitive/PrimitiveQuad.h"

namespace Engine::Component
{
    UQuadComponent::UQuadComponent()
    {
        static CPrimitiveQuad quadPrimitive;
        RenderCommand.MeshData = quadPrimitive.GetMeshData();
    }

    REGISTER_CLASS(Engine::Component, UQuadComponent)
} // namespace Engine::Component