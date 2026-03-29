#pragma once

#include <memory>
#include <utility>

#include "Engine/Asset/Asset.h"
#include "Asset/Cooked/StaticMeshCookedData.h"
#include "Asset/Runtime/StaticMeshRenderResource.h"

using namespace Asset;

class UStaticMesh : public UAsset
{
    DECLARE_RTTI(UStaticMesh, UAsset)

  public:
    const std::shared_ptr<FStaticMeshCookedData>& GetCookedData() const { return CookedData; }

    void SetCookedData(std::shared_ptr<FStaticMeshCookedData> InCookedData)
    {
        CookedData = std::move(InCookedData);
    }

    const std::shared_ptr<FStaticMeshRenderResource>& GetRenderResource() const
    {
        return RenderResource;
    }

    void SetRenderResource(std::shared_ptr<FStaticMeshRenderResource> InRenderResource)
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
    std::shared_ptr<FStaticMeshCookedData>     CookedData;
    std::shared_ptr<FStaticMeshRenderResource> RenderResource;
};
