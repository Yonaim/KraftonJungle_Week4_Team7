#include "PaperSpriteComponent.h"

#include <algorithm>
#include <cfloat>
#include <cstring>
#include <filesystem>

#include "Core/Logging/LogMacros.h"
#include "Engine/Asset/Material.h"
#include "Engine/Asset/StaticMesh.h"
#include "Engine/Component/Core/ComponentProperty.h"
#include "Engine/Game/Actor.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/Primitive/FMeshData.h"
#include "Renderer/RenderAsset/TextureResource.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/SceneView.h"
#include "RHI/D3D11/D3D11Texture.h"
#include "Resources/Mesh/Quad.h"

namespace Engine::Component
{
    namespace
    {
        static bool ReadStaticMeshVertexPosition(const UStaticMesh* InMeshAsset, uint32 VertexIndex,
                                                 FVector& OutPosition)
        {
            if (InMeshAsset == nullptr || !InMeshAsset->IsValidLowLevel())
            {
                return false;
            }

            const TArray<uint8>& VertexData = InMeshAsset->GetVerticesData();
            const uint32         VertexStride = InMeshAsset->GetVertexStride();
            const uint32         VertexCount = InMeshAsset->GetVerticesCount();
            if (VertexStride < sizeof(FVector) || VertexIndex >= VertexCount)
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

    const FString& UPaperSpriteComponent::GetDefaultQuadMeshPath()
    {
        static const FString Path = "/Content/Mesh/Primitive/quad.obj";
        return Path;
    }

    void UPaperSpriteComponent::SetMeshAssetPath(const FString& InPath)
    {
        const FString& NewPath = InPath.empty() ? GetDefaultQuadMeshPath() : InPath;
        if (MeshPath == NewPath)
        {
            return;
        }

        MeshPath = NewPath;
        MeshAsset = nullptr;
        MeshData.reset();
        bBoundsDirty = true;
    }

    void UPaperSpriteComponent::SetMeshAsset(UStaticMesh* InMeshAsset)
    {
        if (MeshAsset == InMeshAsset)
        {
            return;
        }

        MeshAsset = InMeshAsset;
        MeshData.reset();
        bBoundsDirty = true;
    }

    void UPaperSpriteComponent::SetTextureAsset(UTexture* InTextureAsset)
    {
        if (TextureAsset == InTextureAsset)
        {
            return;
        }

        TextureAsset = InTextureAsset;
        Material = nullptr;
        bBoundsDirty = true;
    }

    void UPaperSpriteComponent::SetTexturePath(const FString& InPath)
    {
        if (TexturePath == InPath)
        {
            return;
        }

        TexturePath = InPath;
        TextureAsset = nullptr;
        Material = nullptr;
        bBoundsDirty = true;
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

        FComponentPropertyOptions TexturePathOptions;
        TexturePathOptions.ExpectedAssetPathKind = EComponentAssetPathKind::TextureImage;

        Builder.AddAssetPath(
            "texture", L"Texture", [this]() { return GetTexturePath(); },
            [this](const FString& InPath) { SetTexturePath(InPath); }, TexturePathOptions);

        Builder.AddBool(
            "billboard", L"Billboard", [this]() { return GetBillboard(); },
            [this](bool bInValue) { SetBillboard(bInValue); });
    }

    FVector2 UPaperSpriteComponent::GetSpriteAspectScale() const
    {
        const FTextureRenderResource* TextureResource = GetTextureRenderResource();
        if (TextureResource == nullptr || TextureResource->Width <= 0 ||
            TextureResource->Height <= 0)
        {
            return FVector2(1.0f, 1.0f);
        }

        const float Width = static_cast<float>(TextureResource->Width);
        const float Height = static_cast<float>(TextureResource->Height);
        if (Width <= 0.0f || Height <= 0.0f)
        {
            return FVector2(1.0f, 1.0f);
        }

        if (Width >= Height)
        {
            return FVector2(1.0f, Height / Width);
        }

        return FVector2(Width / Height, 1.0f);
    }

    void UPaperSpriteComponent::EnsureDynamicQuadMeshData() const
    {
        if (!MeshData)
        {
            MeshData = std::make_shared<FMeshData>();
        }

        MeshData->bIsDynamicMesh = true;
        MeshData->Topology = EMeshTopology::EMT_TriangleList;
        MeshData->VertexBuffer.reset();
        MeshData->IndexBuffer.reset();
        MeshData->Vertices = {
            {FVector(-1.0f, -1.0f, 0.0f), FVector(0, 0, 1), FColor::White(), FVector2(0, 1)},
            {FVector(-1.0f, 1.0f, 0.0f), FVector(0, 0, 1), FColor::White(), FVector2(1, 1)},
            {FVector(1.0f, 1.0f, 0.0f), FVector(0, 0, 1), FColor::White(), FVector2(1, 0)},
            {FVector(1.0f, -1.0f, 0.0f), FVector(0, 0, 1), FColor::White(), FVector2(0, 0)},
        };
        MeshData->Indices = {0, 2, 1, 0, 3, 2};
        MeshData->VertexBufferCount = static_cast<uint32>(MeshData->Vertices.size());
        MeshData->IndexBufferCount = static_cast<uint32>(MeshData->Indices.size());
    }

    void UPaperSpriteComponent::EnsureStaticMeshRenderData() const
    {
        if (MeshAsset == nullptr || !MeshAsset->IsValidLowLevel() ||
            MeshAsset->GetRenderResource() == nullptr)
        {
            EnsureDynamicQuadMeshData();
            return;
        }

        if (!MeshData)
        {
            MeshData = std::make_shared<FMeshData>();
        }

        MeshData->bIsDynamicMesh = false;
        MeshData->Topology = EMeshTopology::EMT_TriangleList;
        MeshData->Vertices.clear();
        MeshData->Indices.clear();
        MeshData->VertexBuffer = MeshAsset->GetRenderResource()->VertexBuffer;
        MeshData->IndexBuffer = MeshAsset->GetRenderResource()->IndexBuffer;
        MeshData->VertexBufferCount = MeshAsset->GetVerticesCount();
        MeshData->IndexBufferCount = MeshAsset->GetIndicesCount();
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

        EnsureStaticMeshRenderData();

        if (TextureAsset)
        {
            if (!Material)
            {
                Material = std::make_shared<UMaterial>();
                Material->SetAssetName("M_Sprite_" + TextureAsset->GetAssetName());

                auto CookedData = std::make_shared<FMtlCookedData>();
                CookedData->Name = "M_Sprite_" + TextureAsset->GetAssetName();
                Material->SetCookedData(CookedData);
            }

            std::shared_ptr<FMaterialRenderResource> RenderResource = Material->GetRenderResource();
            if (!RenderResource)
            {
                RenderResource = std::make_shared<FMaterialRenderResource>();
                Material->SetRenderResource(RenderResource);
            }

            RenderResource->BaseColorTexture.reset();
            RenderResource->NormalTexture.reset();
            RenderResource->ORMTexture.reset();

            if (TextureAsset->GetRenderResource() && TextureAsset->GetRenderResource()->GetSRV())
            {
                RHI::FTextureDesc Desc;
                Desc.Width =
                    TextureAsset->GetCookedData() ? TextureAsset->GetCookedData()->Width : 0;
                Desc.Height =
                    TextureAsset->GetCookedData() ? TextureAsset->GetCookedData()->Height : 0;
                Desc.Format = RHI::EPixelFormat::RGBA32F;
                RenderResource->BaseColorTexture = std::make_shared<RHI::D3D11::FD3D11Texture2D>(
                    Desc, nullptr, TextureAsset->GetRenderResource()->GetSRV());
            }
        }
        else
        {
            Material = nullptr;
        }

        FRenderCommand Command;
        Command.MeshData = MeshData.get();
        Command.Material = Material ? Material.get() : FGeneralRenderer::GetDefaultSpriteMaterial();

        const FVector2 SpriteAspectScale = GetSpriteAspectScale();
        const FMatrix  ActorWorld = Actor->GetWorldMatrix();
        const FVector  SpriteOrigin = ActorWorld.GetOrigin() + BillboardOffset;

        if (bBillboard && OutRenderData.SceneView)
        {
            const FMatrix CameraWorld = OutRenderData.SceneView->GetViewMatrix().GetInverse();
            const FVector RightAxis = CameraWorld.GetRightVector();
            const FVector UpAxis = CameraWorld.GetUpVector();
            const FVector ForwardAxis = CameraWorld.GetForwardVector();
            const FVector WorldScale = Actor->GetScale();

            const FVector Row0 = UpAxis * (WorldScale.X * SpriteAspectScale.X);
            const FVector Row1 = RightAxis * (WorldScale.Y * SpriteAspectScale.Y);
            const FVector Row2 = -ForwardAxis * WorldScale.Z;

            Command.WorldMatrix.M[0][0] = Row0.X;
            Command.WorldMatrix.M[0][1] = Row0.Y;
            Command.WorldMatrix.M[0][2] = Row0.Z;
            Command.WorldMatrix.M[0][3] = 0.0f;
            Command.WorldMatrix.M[1][0] = Row1.X;
            Command.WorldMatrix.M[1][1] = Row1.Y;
            Command.WorldMatrix.M[1][2] = Row1.Z;
            Command.WorldMatrix.M[1][3] = 0.0f;
            Command.WorldMatrix.M[2][0] = Row2.X;
            Command.WorldMatrix.M[2][1] = Row2.Y;
            Command.WorldMatrix.M[2][2] = Row2.Z;
            Command.WorldMatrix.M[2][3] = 0.0f;
            Command.WorldMatrix.M[3][0] = SpriteOrigin.X;
            Command.WorldMatrix.M[3][1] = SpriteOrigin.Y;
            Command.WorldMatrix.M[3][2] = SpriteOrigin.Z;
            Command.WorldMatrix.M[3][3] = 1.0f;
        }
        else
        {
            Command.WorldMatrix = ActorWorld;
            Command.WorldMatrix.M[0][0] *= SpriteAspectScale.X;
            Command.WorldMatrix.M[0][1] *= SpriteAspectScale.X;
            Command.WorldMatrix.M[0][2] *= SpriteAspectScale.X;
            Command.WorldMatrix.M[1][0] *= SpriteAspectScale.Y;
            Command.WorldMatrix.M[1][1] *= SpriteAspectScale.Y;
            Command.WorldMatrix.M[1][2] *= SpriteAspectScale.Y;
            Command.WorldMatrix.M[3][0] += BillboardOffset.X;
            Command.WorldMatrix.M[3][1] += BillboardOffset.Y;
            Command.WorldMatrix.M[3][2] += BillboardOffset.Z;
        }

        Command.ObjectId = Actor->GetObjectId();
        Command.bDrawAABB = Actor->IsSelected();
        Command.WorldAABB = GetWorldAABB();
        Command.SetDefaultStates();
        Command.RasterizerOption.CullMode = D3D11_CULL_NONE;
        Command.BlendOption.BlendEnable = true;
        Command.BlendOption.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        Command.BlendOption.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        Command.BlendOption.BlendOp = D3D11_BLEND_OP_ADD;
        Command.BlendOption.SrcBlendAlpha = D3D11_BLEND_ONE;
        Command.BlendOption.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        Command.BlendOption.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        Command.SetStates(Command.Material, MeshData->Topology);
        Command.bIsVisible = Actor->IsVisible();
        Command.bIsPickable = Actor->IsPickable();
        Command.bIsSelected = Actor->IsSelected();
        Command.bIsHovered = Actor->IsHovered();

        OutRenderData.RenderCommands.push_back(Command);
    }

    bool UPaperSpriteComponent::GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const
    {
        OutTriangles.clear();

        const FVector2 SpriteAspectScale = GetSpriteAspectScale();

        if (MeshAsset != nullptr && MeshAsset->IsValidLowLevel())
        {
            const TArray<uint32>& Indices = MeshAsset->GetIndicesData();
            for (size_t i = 0; i + 2 < Indices.size(); i += 3)
            {
                FVector P0, P1, P2;
                if (!ReadStaticMeshVertexPosition(MeshAsset, Indices[i + 0], P0) ||
                    !ReadStaticMeshVertexPosition(MeshAsset, Indices[i + 1], P1) ||
                    !ReadStaticMeshVertexPosition(MeshAsset, Indices[i + 2], P2))
                {
                    continue;
                }

                P0.X *= SpriteAspectScale.X;
                P0.Y *= SpriteAspectScale.Y;
                P1.X *= SpriteAspectScale.X;
                P1.Y *= SpriteAspectScale.Y;
                P2.X *= SpriteAspectScale.X;
                P2.Y *= SpriteAspectScale.Y;

                Geometry::FTriangle Triangle;
                Triangle.V0 = P0;
                Triangle.V1 = P1;
                Triangle.V2 = P2;
                OutTriangles.push_back(Triangle);
            }

            return !OutTriangles.empty();
        }

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
            Triangle.V0 = FVector{quad_vertices[I0].x * SpriteAspectScale.X,
                                  quad_vertices[I0].y * SpriteAspectScale.Y, quad_vertices[I0].z};
            Triangle.V1 = FVector{quad_vertices[I1].x * SpriteAspectScale.X,
                                  quad_vertices[I1].y * SpriteAspectScale.Y, quad_vertices[I1].z};
            Triangle.V2 = FVector{quad_vertices[I2].x * SpriteAspectScale.X,
                                  quad_vertices[I2].y * SpriteAspectScale.Y, quad_vertices[I2].z};

            OutTriangles.push_back(Triangle);
        }

        return OutTriangles.size() > 0;
    }

    Geometry::FAABB UPaperSpriteComponent::GetLocalAABB() const
    {
        const FVector2 SpriteAspectScale = GetSpriteAspectScale();

        if (MeshAsset != nullptr && MeshAsset->IsValidLowLevel())
        {
            Geometry::FAABB MeshAABB = MeshAsset->GetAABB();
            MeshAABB.Min.X *= SpriteAspectScale.X;
            MeshAABB.Max.X *= SpriteAspectScale.X;
            MeshAABB.Min.Y *= SpriteAspectScale.Y;
            MeshAABB.Max.Y *= SpriteAspectScale.Y;
            return MeshAABB;
        }

        FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
        FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (uint32_t i = 0; i < quad_vertex_count; ++i)
        {
            const FVector P(quad_vertices[i].x * SpriteAspectScale.X,
                            quad_vertices[i].y * SpriteAspectScale.Y, quad_vertices[i].z);

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
