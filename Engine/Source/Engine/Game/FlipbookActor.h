#pragma once
#include "SpriteActor.h"

namespace Engine::Component
{
    class UPrimitiveComponent;
    class USubUVAnimatedComponent;
} // namespace Engine::Component

class ENGINE_API AFlipbookActor : public ASpriteActor
{
    DECLARE_RTTI(AFlipbookActor, ASpriteActor)
  public:
    AFlipbookActor();
    ~AFlipbookActor() override = default;

    Engine::Component::USubUVAnimatedComponent* GetSubUVAnimatedComponent() const;
};