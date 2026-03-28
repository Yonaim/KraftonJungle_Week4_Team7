#pragma once
#include "SpriteActor.h"

namespace Engine::Component
{
    class UPrimitiveComponent;
    class USubUVComponent;
}

class ENGINE_API AAtlasSpriteActor : public ASpriteActor
{
    DECLARE_RTTI(AAtlasSpriteActor, ASpriteActor)
public:
    AAtlasSpriteActor();
    ~AAtlasSpriteActor() override = default;
    
    Engine::Component::USubUVComponent * GetSubUVTextureComponent() const;
    
    bool           IsRenderable() const override;
    bool           IsSelected() const override;
    FColor         GetColor() const override;
    EBasicMeshType GetMeshType() const override;
    uint32         GetObjectId() const override;
    
private:
    Engine::Component::UPrimitiveComponent* GetPrimitiveComponent() const;
};

