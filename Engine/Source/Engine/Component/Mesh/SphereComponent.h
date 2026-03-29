#pragma once

#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Renderer/Types/BasicMeshType.h"

namespace Engine::Component
{
    class ENGINE_API USphereComponent : public UPrimitiveComponent
    {
        DECLARE_RTTI(USphereComponent, UPrimitiveComponent)
      public:
        USphereComponent();
        ~USphereComponent() override = default;

        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::Sphere; }

        void CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const override;
    };
} // namespace Engine::Component