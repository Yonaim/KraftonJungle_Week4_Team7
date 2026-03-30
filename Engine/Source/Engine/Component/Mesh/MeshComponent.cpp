#include "Core/CoreMinimal.h"
#include "MeshComponent.h"

#include "Renderer/Primitive/PrimitiveCube.h"

Engine::Component::UMeshComponent::UMeshComponent()
{
    // 임시로 모든 MeshComponent가 Cube로 렌더링되도록 설정.
    // 추후 메시 파일 읽어서 설정하도록 수정
    static CPrimitiveCube cubePrimitive;
    RenderCommand.MeshData = cubePrimitive.GetMeshData();
}