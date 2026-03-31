#include "Core/CoreMinimal.h"
#include "StaticMeshComponent.h"
#include "Engine/Component/Core/ComponentProperty.h"
#include "Engine/Asset/StaticMesh.h"

#include <cstring>

#include "Engine/Game/Actor.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/Primitive/FMeshData.h"
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

    FString UStaticMeshComponent::GetStaticMeshPath() const { return MeshPath; }

    void UStaticMeshComponent::SetStaticMeshPath(const FString& InPath) { MeshPath = InPath; }

    void UStaticMeshComponent::SetStaticMeshAsset(UStaticMesh* InStaticMesh)
    {
        bBoundsDirty = true;
        StaticMesh = InStaticMesh;
        SyncMaterialOverridesWithStaticMesh();
    }

    void UStaticMeshComponent::SyncMaterialOverridesWithStaticMesh()
    {
        OverrideMaterials.clear();

        if (StaticMesh == nullptr)
        {
            return;
        }

        OverrideMaterials.resize(StaticMesh->GetMaterialSlots().size(), nullptr);
    }

    void UStaticMeshComponent::CollectRenderData(FSceneRenderData& OutRenderData,
                                                 ESceneShowFlags   InShowFlags) const
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

        if (StaticMesh == nullptr)
        {
            return;
        }

        // Update FMeshData
        if (!MeshData)
        {
            MeshData = std::make_shared<FMeshData>();
        }

        MeshData->Topology = EMeshTopology::EMT_TriangleList;
        MeshData->VertexBufferCount = StaticMesh->GetVerticesCount();
        MeshData->IndexBufferCount = StaticMesh->GetIndicesCount();
        MeshData->VertexBuffer = StaticMesh->GetRenderResource()->VertexBuffer;
        MeshData->IndexBuffer = StaticMesh->GetRenderResource()->IndexBuffer;
        // DEBUG: 렌더링을 위해 Vertices와 Indices의 count 채우기
        if (MeshData->Vertices.size() != StaticMesh->GetRenderResource()->VertexCount)
        {
            MeshData->Vertices.assign(StaticMesh->GetRenderResource()->VertexCount,
                                      FPrimitiveVertex());
        }
        if (MeshData->Indices.size() != StaticMesh->GetRenderResource()->IndexCount)
        {
            MeshData->Indices.assign(StaticMesh->GetRenderResource()->IndexCount, 0);
        }
        MeshData->bIsDirty = false;

        const TArray<FStaticMeshSection>& Sections = StaticMesh->GetSections();

        if (Sections.empty())
        {
            FRenderCommand Cmd;
            Cmd.MeshData = MeshData.get();
            Cmd.WorldMatrix = GetRelativeMatrix();
            Cmd.ObjectId = Actor->GetObjectId();
            Cmd.bDrawAABB = Actor->IsSelected() || Actor->IsShowBounds();
            Cmd.WorldAABB = GetWorldAABB();
            Cmd.SetDefaultStates();

            Cmd.Material = GetMaterial(0);
            if (Cmd.Material == nullptr)
            {
                Cmd.Material = FGeneralRenderer::GetDefaultMaterial();
            }
            Cmd.SetStates(Cmd.Material, MeshData->Topology);

            Cmd.bIsVisible = Actor->IsVisible();
            Cmd.bIsPickable = Actor->IsPickable();
            Cmd.bIsSelected = Actor->IsSelected();
            Cmd.bIsHovered = Actor->IsHovered();

            OutRenderData.RenderCommands.push_back(Cmd);
        }
        else
        {
            for (const auto& Section : Sections)
            {
                FRenderCommand Cmd;
                Cmd.MeshData = MeshData.get();
                Cmd.WorldMatrix = GetRelativeMatrix();
                Cmd.ObjectId = Actor->GetObjectId();
                Cmd.bDrawAABB = Actor->IsSelected() || Actor->IsShowBounds();
                Cmd.WorldAABB = GetWorldAABB();
                Cmd.SetDefaultStates();

                Cmd.FirstIndex = Section.FirstIndex;
                Cmd.IndexCount = Section.IndexCount;

                Cmd.Material = GetMaterial(Section.MaterialIndex);
                if (Cmd.Material == nullptr)
                {
                    Cmd.Material = FGeneralRenderer::GetDefaultMaterial();
                }
                Cmd.SetStates(Cmd.Material, MeshData->Topology);

                Cmd.bIsVisible = Actor->IsVisible();
                Cmd.bIsPickable = Actor->IsPickable();
                Cmd.bIsSelected = Actor->IsSelected();
                Cmd.bIsHovered = Actor->IsHovered();

                OutRenderData.RenderCommands.push_back(Cmd);
            }
        }
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

    int32 UStaticMeshComponent::GetMaterialSlotCount() const
    {
        if (StaticMesh == nullptr)
        {
            return 0;
        }
        return static_cast<int32>(StaticMesh->GetMaterialSlots().size());
    }

    bool UStaticMeshComponent::IsValidMaterialSlotIndex(int32 SlotIndex) const
    {
        return SlotIndex >= 0 && SlotIndex < GetMaterialSlotCount();
    }

    FString UStaticMeshComponent::GetMaterialSlotName(int32 SlotIndex) const
    {
        if (!IsValidMaterialSlotIndex(SlotIndex) || StaticMesh == nullptr)
        {
            return {};
        }

        const auto& CookedData = StaticMesh->GetCookedData();
        if (CookedData != nullptr && static_cast<size_t>(SlotIndex) < CookedData->Materials.size())
        {
            const Asset::FObjCookedMaterialRef& MaterialRef = CookedData->Materials[SlotIndex];
            if (!MaterialRef.Name.empty())
            {
                return MaterialRef.Name;
            }
        }

        return "Material Slot " + std::to_string(SlotIndex);
    }

    UMaterial* UStaticMeshComponent::GetMaterial(int32 SlotIndex) const
    {
        if (!IsValidMaterialSlotIndex(SlotIndex))
        {
            return nullptr;
        }

        if (UMaterial* OverrideMaterial = GetOverrideMaterial(SlotIndex))
        {
            return OverrideMaterial;
        }

        const auto& Slots = StaticMesh->GetMaterialSlots();
        return Slots[SlotIndex];
    }

    UMaterial* UStaticMeshComponent::GetOverrideMaterial(int32 SlotIndex) const
    {
        if (!IsValidMaterialSlotIndex(SlotIndex))
        {
            return nullptr;
        }

        if (static_cast<size_t>(SlotIndex) >= OverrideMaterials.size())
        {
            return nullptr;
        }

        return OverrideMaterials[SlotIndex];
    }

    bool UStaticMeshComponent::HasMaterialOverride(int32 SlotIndex) const
    {
        return GetOverrideMaterial(SlotIndex) != nullptr;
    }

    void UStaticMeshComponent::SetMaterial(int32 SlotIndex, UMaterial* InMaterial)
    {
        if (!IsValidMaterialSlotIndex(SlotIndex))
        {
            return;
        }

        if (OverrideMaterials.size() < StaticMesh->GetMaterialSlots().size())
        {
            OverrideMaterials.resize(StaticMesh->GetMaterialSlots().size(), nullptr);
        }

        OverrideMaterials[SlotIndex] = InMaterial;
    }

    void UStaticMeshComponent::ClearMaterialOverride(int32 SlotIndex)
    {
        if (!IsValidMaterialSlotIndex(SlotIndex))
        {
            return;
        }

        if (static_cast<size_t>(SlotIndex) >= OverrideMaterials.size())
        {
            return;
        }

        OverrideMaterials[SlotIndex] = nullptr;
    }

    void UStaticMeshComponent::ClearAllMaterialOverrides()
    {
        for (UMaterial*& OverrideMaterial : OverrideMaterials)
        {
            OverrideMaterial = nullptr;
        }
    }
} // namespace Engine::Component