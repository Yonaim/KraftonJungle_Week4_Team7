#pragma once
#include "MeshComponent.h"

class UStaticMesh;

namespace Engine::Component
{
    class ENGINE_API UStaticMeshComponent : public UMeshComponent
    {
        DECLARE_RTTI(UStaticComponent, UMeshComponent)
      public:
        UStaticMeshComponent() = default;
        ~UStaticMeshComponent() override = default;

        FString GetStaticMeshPath() const;
        void    SetStaticMeshPath(const FString& InPath);

        // Primitive
        void DescribeProperties(FComponentPropertyBuilder& Builder) override;
        bool GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const override;

      private:
        FString MeshPath;
      protected:
        UStaticMesh* StaticMesh = nullptr;
    };

} // namespace Engine::Component
