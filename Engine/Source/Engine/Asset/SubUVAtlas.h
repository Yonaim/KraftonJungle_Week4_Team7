#pragma once

#include <memory>
#include <utility>

#include "Asset/Asset.h"
#include "Asset/Data/SubUVAtlasCookedData.h"
#include "Asset/Runtime/SubUVAtlasRenderResource.h"

using namespace Asset;

class USubUVAtlas : public UAsset
{
    DECLARE_RTTI(USubUVAtlas, UAsset)

  public:
    const std::shared_ptr<FSubUVAtlasCookedData>& GetCookedData() const { return CookedData; }

    void SetCookedData(std::shared_ptr<FSubUVAtlasCookedData> InCookedData)
    {
        CookedData = std::move(InCookedData);
    }

    const std::shared_ptr<FSubUVAtlasRenderResource>& GetRenderResource() const
    {
        return RenderResource;
    }

    void SetRenderResource(std::shared_ptr<FSubUVAtlasRenderResource> InRenderResource)
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
    std::shared_ptr<FSubUVAtlasCookedData>     CookedData;
    std::shared_ptr<FSubUVAtlasRenderResource> RenderResource;
};
