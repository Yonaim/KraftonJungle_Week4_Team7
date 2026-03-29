#pragma once

#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Renderer/Types/BasicMeshType.h"

namespace Engine::Component
{
    class ENGINE_API UCubeComponent : public UPrimitiveComponent
    {
        DECLARE_RTTI(UCubeComponent, UPrimitiveComponent)
    public:
        UCubeComponent();
        ~UCubeComponent() override = default;

        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::Cube; }
        
        // 테스트용 오버라이딩
        void CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const override;
    };
} // namespace Engine::Component