#pragma once

#include "Engine/Component/Text/TextRenderComponent.h"

class AActor;

namespace Engine::Component
{
    class ENGINE_API UAtlasTextComponent : public UTextRenderComponent
    {
        DECLARE_RTTI(UAtlasTextComponent, UTextRenderComponent)

      public:
        UAtlasTextComponent() = default;
        ~UAtlasTextComponent() override = default;
    };
} // namespace Engine::Component
