#include "PaperSpriteComponent.h"

#include <filesystem>

#include "Engine/Component/Core/ComponentProperty.h"
#include "Resources/Mesh/Quad.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/SceneView.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/Material/Material.h"
#include "Engine/Game/Actor.h"
#include "SceneIO/SceneAssetPath.h"

namespace Engine::Component
{
    void UPaperSpriteComponent::SetTextureAsset(UTexture* InTextureAsset)
    {
        TextureAsset = InTextureAsset;
        Material = nullptr; // Force material recreation
    }

    const FTextureRenderResource* UPaperSpriteComponent::GetTextureRenderResource() const
    {
        return (TextureAsset != nullptr && TextureAsset->GetRenderResource())
                   ? TextureAsset->GetRenderResource().get()
                   : nullptr;
    }

    FTextureRenderResource* UPaperSpriteComponent::GetTextureRenderResource()
    {
        return (TextureAsset != nullptr && TextureAsset->GetRenderResource())
                   ? TextureAsset->GetRenderResource().get()
                   : nullptr;
    }

    void UPaperSpriteComponent::SetBillboard(bool bInBillboard) { bBillboard = bInBillboard; }

    void UPaperSpriteComponent::SetBillboardOffset(const FVector& InBillboardOffset)
    {
        BillboardOffset = InBillboardOffset;
    }

    void UPaperSpriteComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        UMeshComponent::DescribeProperties(Builder);

        Builder.AddBool(
            "billboard", L"Billboard", [this]() { return GetBillboard(); },
            [this](bool bInValue) { SetBillboard(bInValue); });
    }

    void UPaperSpriteComponent::CollectRenderData(FSceneRenderData& OutRenderData,
                                                  ESceneShowFlags   InShowFlags) const
    {
        if (!IsFlagSet(InShowFlags, ESceneShowFlags::SF_Sprites))
        {
            return;
        }

        AActor* Actor = GetOwnerActor();
        if (Actor == nullptr)
        {
            return;
        }

        if (!MeshData)
        {
            MeshData = std::make_shared<FMeshData>();
            MeshData->Topology = EMeshTopology::EMT_TriangleList;
            
            // Create a quad on XY plane (Unreal style: X=Forward/Height, Y=Right/Width)
            MeshData->Vertices = {
                { FVector(-1.0f, -1.0f, 0.0f), FColor::White(), FVector(0,0,1), FVector2(0, 1) }, // Bottom-Left
                { FVector(-1.0f,  1.0f, 0.0f), FColor::White(), FVector(0,0,1), FVector2(1, 1) }, // Bottom-Right
                { FVector( 1.0f,  1.0f, 0.0f), FColor::White(), FVector(0,0,1), FVector2(1, 0) }, // Top-Right
                { FVector( 1.0f, -1.0f, 0.0f), FColor::White(), FVector(0,0,1), FVector2(0, 0) }  // Top-Left
            };
            MeshData->Indices = { 0, 2, 1, 0, 3, 2 }; 
        }

        if (!Material && TextureAsset)
        {
            // Create a dynamic material based on default sprite material and set texture
            Material = FGeneralRenderer::GetDefaultSpriteMaterial()->CreateDynamicMaterial();
            
            if (TextureAsset->GetRenderResource()->GetSRV())
            {
                auto Tex = std::make_shared<FMaterialTexture>();
                Tex->TextureSRV = TextureAsset->GetRenderResource()->GetSRV();
                Material->SetMaterialTexture(Tex);
            }
        }

        FRenderCommand Command;
        Command.MeshData = MeshData.get();
        Command.Material = Material ? Material.get() : FGeneralRenderer::GetDefaultSpriteMaterial();
        
        const FMatrix ActorWorld = Actor->GetWorldMatrix();
        FVector SpriteOrigin = ActorWorld.GetOrigin() + BillboardOffset;

        if (bBillboard && OutRenderData.SceneView)
        {
            const FMatrix CameraWorld = OutRenderData.SceneView->GetViewMatrix().GetInverse();
            FVector RightAxis = CameraWorld.GetRightVector();
            FVector UpAxis = CameraWorld.GetUpVector();
            FVector ForwardAxis = CameraWorld.GetForwardVector();

            const FVector WorldScale = Actor->GetScale();
            
            // Map axes so that the XY-plane quad stands upright and faces camera:
            // Local X (Row 0) = Camera Up (matches texture vertical direction)
            // Local Y (Row 1) = Camera Right (matches texture horizontal direction)
            // Local Z (Row 2) = Camera Back (Normal faces camera)
            FVector Row0 = UpAxis * WorldScale.X;
            FVector Row1 = RightAxis * WorldScale.Y;
            FVector Row2 = -ForwardAxis; 


            Command.WorldMatrix.M[0][0] = Row0.X; Command.WorldMatrix.M[0][1] = Row0.Y; Command.WorldMatrix.M[0][2] = Row0.Z; Command.WorldMatrix.M[0][3] = 0.0f;
            Command.WorldMatrix.M[1][0] = Row1.X; Command.WorldMatrix.M[1][1] = Row1.Y; Command.WorldMatrix.M[1][2] = Row1.Z; Command.WorldMatrix.M[1][3] = 0.0f;
            Command.WorldMatrix.M[2][0] = Row2.X; Command.WorldMatrix.M[2][1] = Row2.Y; Command.WorldMatrix.M[2][2] = Row2.Z; Command.WorldMatrix.M[2][3] = 0.0f;
            Command.WorldMatrix.M[3][0] = SpriteOrigin.X; Command.WorldMatrix.M[3][1] = SpriteOrigin.Y; Command.WorldMatrix.M[3][2] = SpriteOrigin.Z; Command.WorldMatrix.M[3][3] = 1.0f;
        }
        else
        {
            Command.WorldMatrix = ActorWorld;
            // Apply world-space offset directly to the translation row
            Command.WorldMatrix.M[3][0] += BillboardOffset.X;
            Command.WorldMatrix.M[3][1] += BillboardOffset.Y;
            Command.WorldMatrix.M[3][2] += BillboardOffset.Z;
        }

        Command.ObjectId = Actor->GetObjectId();
        Command.bDrawAABB = Actor->IsSelected();
        Command.WorldAABB = GetWorldAABB();
        OutRenderData.RenderCommands.push_back(Command);
    }

    bool UPaperSpriteComponent::GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const
    {
        OutTriangles.clear();

        if (quad_topology != EMeshPrimitiveTopology::TriangleList)
        {
            return false;
        }

        for (uint32_t i = 0; i + 2 < quad_index_count; i += 3)
        {
            const uint16_t I0 = quad_indices[i + 0];
            const uint16_t I1 = quad_indices[i + 1];
            const uint16_t I2 = quad_indices[i + 2];

            if (I0 >= quad_vertex_count || I1 >= quad_vertex_count || I2 >= quad_vertex_count)
            {
                continue;
            }

            Geometry::FTriangle Triangle;
            Triangle.V0 = FVector{quad_vertices[I0].x, quad_vertices[I0].y, quad_vertices[I0].z};
            Triangle.V1 = FVector{quad_vertices[I1].x, quad_vertices[I1].y, quad_vertices[I1].z};
            Triangle.V2 = FVector{quad_vertices[I2].x, quad_vertices[I2].y, quad_vertices[I2].z};

            OutTriangles.push_back(Triangle);
        }

        return OutTriangles.size() > 0;
    }

    Geometry::FAABB UPaperSpriteComponent::GetLocalAABB() const
    {
        FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
        FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (uint32_t i = 0; i < quad_vertex_count; ++i)
        {
            const FVector P(quad_vertices[i].x, quad_vertices[i].y, quad_vertices[i].z);

            Min.X = std::min(Min.X, P.X);
            Min.Y = std::min(Min.Y, P.Y);
            Min.Z = std::min(Min.Z, P.Z);

            Max.X = std::max(Max.X, P.X);
            Max.Y = std::max(Max.Y, P.Y);
            Max.Z = std::max(Max.Z, P.Z);
        }

        return Geometry::FAABB(Min, Max);
    }

    REGISTER_CLASS(Engine::Component, UPaperSpriteComponent)
} // namespace Engine::Component
