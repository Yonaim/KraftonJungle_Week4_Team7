#pragma once
#include "MeshComponent.h"
#include "Renderer/Types/BasicMeshType.h"
#include "Engine/Asset/Material.h"

class UStaticMesh;

namespace Engine::Component
{
    class ENGINE_API UStaticMeshComponent : public UMeshComponent
    {
        DECLARE_RTTI(UStaticMeshComponent, UMeshComponent)
      public:
        UStaticMeshComponent() = default;
        ~UStaticMeshComponent() override = default;

        FString GetStaticMeshPath() const;
        void    SetStaticMeshPath(const FString& InPath);

        const UStaticMesh* GetStaticMeshAsset() const { return StaticMesh; }
        UStaticMesh*       GetStaticMeshAsset() { return StaticMesh; }
        void SetStaticMeshAsset(UStaticMesh* InStaticMesh) { StaticMesh = InStaticMesh; }

        // Primitive
        virtual void CollectRenderData(FSceneRenderData& OutRenderData,
                                       ESceneShowFlags   InShowFlags) const override;

        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::None; }

        void DescribeProperties(FComponentPropertyBuilder& Builder) override;
        bool GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const override;

        int32      GetMaterialSlotCount() const;
        UMaterial* GetMaterial(int32 SlotIndex) const;
        void       SetMaterial(int32 SlotIndex, UMaterial* InMaterial);

      protected:
        Geometry::FAABB GetLocalAABB() const override;

      private:
        FString MeshPath;

      protected:
        UStaticMesh* StaticMesh = nullptr;
    };

} // namespace Engine::Component
