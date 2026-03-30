#include "ConeComponent.h"

#include "NewRenderer/Primitive/PrimitiveCone.h"

namespace Engine::Component
{
    UConeComponent::UConeComponent()
    {
        static CPrimitiveCone conePrimitive;
        RenderCommand.MeshData = conePrimitive.GetMeshData();
    }

    REGISTER_CLASS(Engine::Component, UConeComponent)
} // namespace Engine::Component
