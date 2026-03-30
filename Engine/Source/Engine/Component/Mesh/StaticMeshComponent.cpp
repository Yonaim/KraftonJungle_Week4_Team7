#include "Core/CoreMinimal.h"
#include "StaticMeshComponent.h"
#include "Engine/Component/Core/ComponentProperty.h"
#include "Engine/Asset/StaticMesh.h"

#include <cstring>

#include "Engine/Game/Actor.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "RHI/D3D11/D3D11Buffer.h"

namespace Engine::Component
{
    REGISTER_CLASS(Engine::Component, UStaticMeshComponent)

    namespace
    {
        static bool ReadVertexPosition(const TArray<uint8>& VertexData, uint32 VertexStride,
                                       uint32 VertexIndex, FVector& OutPosition)
        {
            if (VertexStride < sizeof(FVector))
            {
                return false;
            }

            const size_t Offset = static_cast<size_t>(VertexIndex) * VertexStride;
            if (Offset + sizeof(FVector) > VertexData.size())
            {
                return false;
            }

            std::memcpy(&OutPosition, VertexData.data() + Offset, sizeof(FVector));
            return true;
        }
    } // namespace

    FString UStaticMeshComponent::GetStaticMeshPath() const
    {
        return MeshPath;
    }

    void UStaticMeshComponent::SetStaticMeshPath(const FString& InPath) { MeshPath = InPath; }

    void UStaticMeshComponent::CollectRenderData(FSceneRenderData& OutRenderData,
        ESceneShowFlags InShowFlags) const
    {
        if (!IsFlagSet(InShowFlags, ESceneShowFlags::SF_Primitives))
        {
            return;
        }

        AActor* Actor = GetOwnerActor();
        if (Actor == nullptr)
        {
            return;
        }

        FRenderCommand& MutableRenderCommand = const_cast<FRenderCommand&>(RenderCommand);
        
        // Generate FMeshData
        FMeshData* MeshData = new FMeshData();
        if (StaticMesh)
        {
            MeshData->Topology = EMeshTopology::EMT_TriangleList;
            MeshData->VertexBuffer = std::static_pointer_cast<RHI::D3D11::FD3D11VertexBuffer>(StaticMesh->GetRenderResource()->VertexBuffer)->GetBuffer(); 
            MeshData->IndexBuffer = std::static_pointer_cast<RHI::D3D11::FD3D11IndexBuffer>(StaticMesh->GetRenderResource()->IndexBuffer)->GetBuffer();
            // DEBUG: 렌더링을 위해 Vertices와 Indices의 count 채우기
            MeshData->Vertices.insert(MeshData->Vertices.begin(), 
                                    StaticMesh->GetRenderResource()->VertexCount,
                                    FPrimitiveVertex()
                                    );
            MeshData->Indices.insert(MeshData->Indices.begin(), 
                                    StaticMesh->GetRenderResource()->IndexCount,
                                    0
                                    );
            MeshData->bIsDirty = false;
        }
        MutableRenderCommand.MeshData = MeshData;
        if (MutableRenderCommand.MeshData == nullptr)
        {
            return;
        }

        if (MutableRenderCommand.Material == nullptr)
        {
            MutableRenderCommand.Material = FGeneralRenderer::GetDefaultMaterial();
        }

        MutableRenderCommand.WorldMatrix = GetRelativeMatrix();
        MutableRenderCommand.ObjectId = Actor->GetObjectId();
        MutableRenderCommand.bDrawAABB = Actor->IsSelected() || Actor->IsShowBounds();
        MutableRenderCommand.WorldAABB = GetWorldAABB();

        MutableRenderCommand.bIsVisible = Actor->IsVisible();
        MutableRenderCommand.bIsPickable = Actor->IsPickable();
        MutableRenderCommand.bIsSelected = Actor->IsSelected();
        MutableRenderCommand.bIsHovered = Actor->IsHovered();

        OutRenderData.RenderCommands.push_back(MutableRenderCommand);
    }

    void UStaticMeshComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        Builder.AddAssetPath(
            "ObjStaticMeshAsset", L"Mesh Asset", [this]() { return GetStaticMeshPath(); },
            [this](const FString& InValue) { SetStaticMeshPath(InValue); });
    }

    bool UStaticMeshComponent::GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const
    {
        OutTriangles.clear();

        if (StaticMesh == nullptr || !StaticMesh->IsValidLowLevel())
        {
            return false;
        }

        const TArray<uint8>&  VertexData = StaticMesh->GetVerticesData();
        const TArray<uint32>& Indices = StaticMesh->GetIndicesData();
        const uint32          VertexStride = StaticMesh->GetVertexStride();
        const uint32          VertexCount = StaticMesh->GetVerticesCount();

        if (VertexStride < sizeof(FVector) || VertexCount == 0)
        {
            return false;
        }

        OutTriangles.reserve(Indices.size() / 3);

        for (size_t i = 0; i + 2 < Indices.size(); i += 3)
        {
            const uint32 I0 = Indices[i + 0];
            const uint32 I1 = Indices[i + 1];
            const uint32 I2 = Indices[i + 2];

            if (I0 >= VertexCount || I1 >= VertexCount || I2 >= VertexCount)
            {
                continue;
            }

            FVector P0, P1, P2;
            if (!ReadVertexPosition(VertexData, VertexStride, I0, P0) ||
                !ReadVertexPosition(VertexData, VertexStride, I1, P1) ||
                !ReadVertexPosition(VertexData, VertexStride, I2, P2))
            {
                continue;
            }

            Geometry::FTriangle Triangle;
            Triangle.V0 = P0;
            Triangle.V1 = P1;
            Triangle.V2 = P2;
            OutTriangles.push_back(Triangle);
        }

        return !OutTriangles.empty();
    }

    Geometry::FAABB UStaticMeshComponent::GetLocalAABB() const
    {
        if (StaticMesh)
        {
            return StaticMesh->GetAABB();
        }

        return {};
    }
} // namespace Engine::Component