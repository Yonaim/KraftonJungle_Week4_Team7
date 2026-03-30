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

    void UCubeComponent::CollectRenderData(FSceneRenderData& OutRenderData,
        ESceneShowFlags InShowFlags) const
    {
        FRenderCommand& MutableRenderCommand = const_cast<FRenderCommand&>(RenderCommand);
        MutableRenderCommand.Material = FGeneralRenderer::GetDefaultMaterial();
        MutableRenderCommand.WorldMatrix = GetRelativeMatrix();
        MutableRenderCommand.ObjectId = GetOwnerActor()->GetObjectId();
        MutableRenderCommand.bDrawAABB = GetOwnerActor()->IsSelected();
        MutableRenderCommand.WorldAABB = GetWorldAABB();
        OutRenderData.RenderCommands.push_back(MutableRenderCommand);
    }

    REGISTER_CLASS(Engine::Component, UCubeComponent)
} // namespace Engine::Component