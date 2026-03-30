#include "QuadComponent.h"

#include "NewRenderer/Primitive/PrimitiveQuad.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/SceneRenderData.h"
#include "Engine/Game/Actor.h"

namespace Engine::Component
{
    UQuadComponent::UQuadComponent()
    {
        static CPrimitiveQuad quadPrimitive;
        RenderCommand.MeshData = quadPrimitive.GetMeshData();
    }

    void UQuadComponent::CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const
    {
        FRenderCommand& MutableRenderCommand = const_cast<FRenderCommand&>(RenderCommand);
        MutableRenderCommand.Material = FGeneralRenderer::GetDefaultMaterial();
        MutableRenderCommand.WorldMatrix = GetRelativeMatrix();
        MutableRenderCommand.ObjectId = GetOwnerActor()->GetObjectId();
        MutableRenderCommand.bDrawAABB = GetOwnerActor()->IsSelected();
        MutableRenderCommand.WorldAABB = GetWorldAABB();
        OutRenderData.RenderCommands.push_back(MutableRenderCommand);
    }

    REGISTER_CLASS(Engine::Component, UQuadComponent)
} // namespace Engine::Component