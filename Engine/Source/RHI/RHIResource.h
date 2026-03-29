#pragma once

#include "Core/CoreMinimal.h"

namespace RHI
{

    // ======================== Base Resource ==========================

    class FRHIResource
    {
      public:
        virtual ~FRHIResource() = default;
    };

} // namespace RHI
