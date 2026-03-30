#pragma once

#include <memory>
#include <utility>

#include "Engine/Asset/Asset.h"
#include "Asset/Cooked/MtlCookedData.h"
#include "Asset/Runtime/MaterialRenderResource.h"

using namespace Asset;

class UMaterial : public UAsset
{
    DECLARE_RTTI(UMaterial, UAsset)

  public:
    const std::shared_ptr<FMtlCookedData>& GetCookedData() const { return CookedData; }

    void SetCookedData(std::shared_ptr<FMtlCookedData> InCookedData)
    {
        CookedData = std::move(InCookedData);
    }

    const std::shared_ptr<FMaterialRenderResource>& GetRenderResource() const
    {
        return RenderResource;
    }

    void SetRenderResource(std::shared_ptr<FMaterialRenderResource> InRenderResource)
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
    std::shared_ptr<FMtlCookedData>     CookedData;
    std::shared_ptr<FMaterialRenderResource> RenderResource;
};
