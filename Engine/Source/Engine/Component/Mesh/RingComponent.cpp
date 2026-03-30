#include "RingComponent.h"

#include "NewRenderer/Primitive/PrimitiveRing.h"

namespace Engine::Component
{
    URingComponent::URingComponent()
    {
        static CPrimitiveRing ringPrimitive;
        RenderCommand.MeshData = ringPrimitive.GetMeshData();
    }

    REGISTER_CLASS(Engine::Component, URingComponent)
} // namespace Engine::Component
