#include "RingComponent.h"

#include "NewRenderer/Primitive/PrimitiveRing.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/SceneRenderData.h"
#include "Engine/Game/Actor.h"

namespace Engine::Component
{
    URingComponent::URingComponent()
    {
        static CPrimitiveRing ringPrimitive;
        RenderCommand.MeshData = ringPrimitive.GetMeshData();
    }

    void URingComponent::CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const
    {
        FRenderCommand& MutableRenderCommand = const_cast<FRenderCommand&>(RenderCommand);
        MutableRenderCommand.Material = FGeneralRenderer::GetDefaultMaterial();
        MutableRenderCommand.WorldMatrix = GetRelativeMatrix();
        MutableRenderCommand.bDrawAABB = GetOwnerActor()->IsSelected();
        MutableRenderCommand.WorldAABB = GetWorldAABB();
        OutRenderData.RenderCommands.push_back(MutableRenderCommand);
    }

    REGISTER_CLASS(Engine::Component, URingComponent)
} // namespace Engine::Component
