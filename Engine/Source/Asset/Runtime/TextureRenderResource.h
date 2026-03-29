#pragma once

#include <memory>

#include "Asset/Cooked/TextureCookedData.h"
#include "RHI/DynamicRHI.h"
#include "RHI/RHITexture.h"

namespace Asset
{
    struct FTextureRenderResource
    {
        std::shared_ptr<RHI::FRHITexture> Texture;

        static std::shared_ptr<FTextureRenderResource> Create(const FTextureCookedData& CookedData,
                                                              RHI::FDynamicRHI&         RHI);

        bool IsValid() const;
        void Reset();

      private:
        static RHI::EPixelFormat ResolvePixelFormat(uint32 Channels);
    };
} // namespace Asset
