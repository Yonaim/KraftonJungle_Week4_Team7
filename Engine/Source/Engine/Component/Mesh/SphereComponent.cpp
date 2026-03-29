#include "SphereComponent.h"

#include "NewRenderer/Primitive/PrimitiveSphere.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/SceneRenderData.h"
#include "Engine/Game/Actor.h"

namespace Engine::Component
{
    USphereComponent::USphereComponent()
    {
        static CPrimitiveSphere spherePrimitive;
        RenderCommand.MeshData = spherePrimitive.GetMeshData();
    }

    void USphereComponent::CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const
    {
        FRenderCommand& MutableRenderCommand = const_cast<FRenderCommand&>(RenderCommand);
        MutableRenderCommand.Material = FGeneralRenderer::GetDefaultMaterial();
        MutableRenderCommand.WorldMatrix = GetRelativeMatrix();
        MutableRenderCommand.bDrawAABB = GetOwnerActor()->IsSelected();
        MutableRenderCommand.WorldAABB = GetWorldAABB();
        OutRenderData.RenderCommands.push_back(MutableRenderCommand);
    }

    REGISTER_CLASS(Engine::Component, USphereComponent)
} // namespace Engine::Component