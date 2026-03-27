#pragma once
#include "UMeshComponent.h"

namespace Engine::Component
{
    class ENGINE_API UStaticMeshComponent : public UMeshComponent
    {
        DECLARE_RTTI(UStaticComponent, UMeshComponent)
      public:
        UStaticMeshComponent() = default;
        ~UStaticMeshComponent() override = default;

      protected:
    };

} 
