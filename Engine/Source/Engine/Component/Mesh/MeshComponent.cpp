#include "Core/CoreMinimal.h"
#include "MeshComponent.h"

#include "Engine/Game/Actor.h"
#include "NewRenderer/Primitive/PrimitiveCube.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/D3D11/GeneralRenderer.h"

Engine::Component::UMeshComponent::UMeshComponent()
{
    // 임시로 모든 MeshComponent가 Cube로 렌더링되도록 설정.
    // 추후 메시 파일 읽어서 설정하도록 수정
    static CPrimitiveCube cubePrimitive;
    RenderCommand.MeshData = cubePrimitive.GetMeshData();
}

void Engine::Component::UMeshComponent::CollectRenderData(FSceneRenderData& OutRenderData,
                                                          ESceneShowFlags InShowFlags) const
{
    FRenderCommand& MutableRenderCommand = const_cast<FRenderCommand&>(RenderCommand);
    MutableRenderCommand.Material = FGeneralRenderer::GetDefaultMaterial();
    MutableRenderCommand.WorldMatrix = GetRelativeMatrix();
    MutableRenderCommand.bDrawAABB = GetOwnerActor()->IsSelected();
    MutableRenderCommand.WorldAABB = GetWorldAABB();
    OutRenderData.RenderCommands.push_back(MutableRenderCommand);
}