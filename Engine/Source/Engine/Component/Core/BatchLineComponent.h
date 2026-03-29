#pragma once

#include "Engine/Component/Core/SceneComponent.h"

// TODO
namespace Engine::Component
{
    class ENGINE_API UBatchLineComponent : public USceneComponent
    {
        DECLARE_RTTI(UBatchLineComponent, USceneComponent)

      public:
        UBatchLineComponent() = default;
        ~UBatchLineComponent() override = default;

        bool ShouldSerializeInScene() const override { return false; }
    };
} // namespace Engine::Component
