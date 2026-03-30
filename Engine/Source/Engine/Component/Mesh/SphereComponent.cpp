#include "SphereComponent.h"

#include "NewRenderer/Primitive/PrimitiveSphere.h"

namespace Engine::Component
{
    USphereComponent::USphereComponent()
    {
        static CPrimitiveSphere spherePrimitive;
        RenderCommand.MeshData = spherePrimitive.GetMeshData();
    }

    REGISTER_CLASS(Engine::Component, USphereComponent)
} // namespace Engine::Component