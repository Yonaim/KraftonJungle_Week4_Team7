#pragma once

#include "SpriteActor.h"

namespace Engine::Component
{
    class UPrimitiveComponent;
    class USubUVAnimatedComponent;
}

class ENGINE_API AEffectActor : public ASpriteActor
{
    DECLARE_RTTI(AEffectActor, ASpriteActor)

  public:
    AEffectActor();
    ~AEffectActor() override = default;

    Engine::Component::USubUVAnimatedComponent* GetEffectComponent() const;
};
