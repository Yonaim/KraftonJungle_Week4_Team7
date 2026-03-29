#pragma once

#include <memory>

#include "Asset/Cooked/SubUVAtlasCookedData.h"
#include "RHI/DynamicRHI.h"
#include "RHI/RHITexture.h"

namespace Asset
{
    struct FSubUVAtlasRenderResource
    {
        std::shared_ptr<RHI::FRHITexture> AtlasTexture;

        static std::shared_ptr<FSubUVAtlasRenderResource>
        Create(const FSubUVAtlasCookedData& CookedData, RHI::FDynamicRHI& RHI);

        bool IsValid() const;
        void Reset();
    };
} // namespace Asset
