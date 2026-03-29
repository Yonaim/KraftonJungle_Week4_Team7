#pragma once

#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Renderer/Types/BasicMeshType.h"

namespace Engine::Component
{
    class ENGINE_API UQuadComponent : public UPrimitiveComponent
    {
        DECLARE_RTTI(UQuadComponent, UPrimitiveComponent)
      public:
        UQuadComponent();
        ~UQuadComponent() override = default;

        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::Quad; }

        void CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const override;
    };
} // namespace Engine::Component