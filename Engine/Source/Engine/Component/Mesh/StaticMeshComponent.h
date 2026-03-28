#pragma once
#include "MeshComponent.h"
#include "Renderer/Types/BasicMeshType.h"

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

        // Primitive
        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::None; }

        void DescribeProperties(FComponentPropertyBuilder& Builder) override;
        bool GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const override;

      protected:
        Geometry::FAABB GetLocalAABB() const override;  
    private:
        FString MeshPath;

      protected:
        UStaticMesh* StaticMesh = nullptr;
    };

} // namespace Engine::Component
