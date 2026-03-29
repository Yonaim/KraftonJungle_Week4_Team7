#pragma once

#include <memory>

#include "Asset/Cooked/FontAtlasCookedData.h"
#include "RHI/DynamicRHI.h"
#include "RHI/RHITexture.h"

namespace Asset
{
    struct FFontAtlasRenderResource
    {
        std::shared_ptr<RHI::FRHITexture> AtlasTexture;

        static std::shared_ptr<FFontAtlasRenderResource>
        Create(const FFontAtlasCookedData& CookedData, RHI::FDynamicRHI& RHI);

        bool IsValid() const;
        void Reset();
    };
} // namespace Asset
