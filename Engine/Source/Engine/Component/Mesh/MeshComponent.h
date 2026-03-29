#pragma once
#include "Engine/Component/Core/PrimitiveComponent.h"

/*
    UMeshComponent(계층 구조용 뼈대)
*/
namespace Engine::Component
{
    
    class ENGINE_API UMeshComponent : public UPrimitiveComponent
    {
        DECLARE_RTTI(UMeshComponent, UPrimitiveComponent)
      public:
        UMeshComponent();
        ~UMeshComponent() override = default;
        
        // Gereral Renderer 호환용으로 만든 임시 override
        void CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const override;
    };
};
