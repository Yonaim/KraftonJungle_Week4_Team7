#pragma once

#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Renderer/Types/BasicMeshType.h"

namespace Engine::Component
{
    class ENGINE_API UCylinderComponent : public UPrimitiveComponent
    {
        DECLARE_RTTI(UCylinderComponent, UPrimitiveComponent)
      public:
        UCylinderComponent();
        ~UCylinderComponent() override = default;

        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::Cylinder; }

        void CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const override;
    };
} // namespace Engine::Component
