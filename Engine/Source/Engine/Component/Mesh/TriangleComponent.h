#pragma once

#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Renderer/Types/BasicMeshType.h"

enum class EBasicMeshType : uint8;

namespace Engine::Component
{
    class ENGINE_API UTriangleComponent : public UPrimitiveComponent
    {
        DECLARE_RTTI(UTriangleComponent, UPrimitiveComponent)
      public:
        UTriangleComponent();
        virtual ~UTriangleComponent() override = default;

        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::Triangle; }

        void CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const override;
    };
} // namespace Engine::Component
