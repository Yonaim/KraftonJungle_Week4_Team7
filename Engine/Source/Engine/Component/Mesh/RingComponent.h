#pragma once

#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Renderer/Types/BasicMeshType.h"

namespace Engine::Component
{
    class ENGINE_API URingComponent : public UPrimitiveComponent
    {
        DECLARE_RTTI(URingComponent, UPrimitiveComponent)
      public:
        URingComponent();
        ~URingComponent() override = default;

        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::Ring; }

        void CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const override;
    };
} // namespace Engine::Component
