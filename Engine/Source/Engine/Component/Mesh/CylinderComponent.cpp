#include "CylinderComponent.h"

#include "NewRenderer/Primitive/PrimitiveCylinder.h"

namespace Engine::Component
{
    UCylinderComponent::UCylinderComponent()
    {
        static CPrimitiveCylinder cylinderPrimitive;
        RenderCommand.MeshData = cylinderPrimitive.GetMeshData();
    }

    REGISTER_CLASS(Engine::Component, UCylinderComponent)
} // namespace Engine::Component
