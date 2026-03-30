#include "TriangleComponent.h"

#include "NewRenderer/Primitive/PrimitiveTriangle.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/SceneRenderData.h"
#include "Engine/Game/Actor.h"

namespace Engine::Component
{
    UTriangleComponent::UTriangleComponent()
    {
        static CPrimitiveTriangle trianglePrimitive;
        RenderCommand.MeshData = trianglePrimitive.GetMeshData();
    }

    void UTriangleComponent::CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const
    {
        FRenderCommand& MutableRenderCommand = const_cast<FRenderCommand&>(RenderCommand);
        MutableRenderCommand.Material = FGeneralRenderer::GetDefaultMaterial();
        MutableRenderCommand.WorldMatrix = GetRelativeMatrix();
        MutableRenderCommand.ObjectId = GetOwnerActor()->GetObjectId();
        MutableRenderCommand.bDrawAABB = GetOwnerActor()->IsSelected();
        MutableRenderCommand.WorldAABB = GetWorldAABB();
        OutRenderData.RenderCommands.push_back(MutableRenderCommand);
    }

    REGISTER_CLASS(Engine::Component, UTriangleComponent)
} // namespace Engine::Component
