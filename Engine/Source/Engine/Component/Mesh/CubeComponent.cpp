#include "CubeComponent.h"

#include "Renderer/SceneRenderData.h"
#include "NewRenderer/Primitive/PrimitiveCube.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include <memory>

#include "Engine/Game/Actor.h"

namespace Engine::Component
{
    UCubeComponent::UCubeComponent()
    {
        static CPrimitiveCube cubePrimitive;
        RenderCommand.MeshData = cubePrimitive.GetMeshData();
    }

    REGISTER_CLASS(Engine::Component, UCubeComponent)
} // namespace Engine::Component