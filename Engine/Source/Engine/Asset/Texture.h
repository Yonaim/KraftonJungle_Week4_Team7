#pragma once

#include <memory>
#include <utility>

#include "Asset/Asset.h"
#include "Asset/Data/TextureCookedData.h"
#include "Asset/Runtime/TextureRenderResource.h"

using namespace Asset;

class UTexture : public UAsset
{
    DECLARE_RTTI(UTexture, UAsset)

  public:
    const std::shared_ptr<FTextureCookedData>& GetCookedData() const { return CookedData; }

    void SetCookedData(std::shared_ptr<FTextureCookedData> InCookedData)
    {
        CookedData = std::move(InCookedData);
    }

    const std::shared_ptr<FTextureRenderResource>& GetRenderResource() const
    {
        return RenderResource;
    }

    void SetRenderResource(std::shared_ptr<FTextureRenderResource> InRenderResource)
    {
        RenderResource = std::move(InRenderResource);
    }

    void ResetRenderResource()
    {
        if (RenderResource)
        {
            RenderResource->Reset();
        }
        RenderResource.reset();
    }

  private:
    std::shared_ptr<FTextureCookedData>     CookedData;
    std::shared_ptr<FTextureRenderResource> RenderResource;
};
