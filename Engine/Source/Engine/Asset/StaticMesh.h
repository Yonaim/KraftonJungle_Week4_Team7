#pragma once

#include <memory>

#include "Asset/Data/StaticMeshCookedData.h"
#include "Engine/Asset/Asset.h"
#include "Renderer/Resource/StaticMeshResource.h"

class UStaticMesh : public UAsset
{
    DECLARE_RTTI(UStaticMesh, UAsset)

  public:
    void SetCookedData(const std::shared_ptr<FStaticMeshCookedData>& InCookedData)
    {
        CookedData = InCookedData;
    }
    void SetResource(const std::shared_ptr<FStaticMeshResource>& InResource)
    {
        Resource = InResource;
    }

    std::shared_ptr<FStaticMeshCookedData> GetCookedData() const { return CookedData; }
    std::shared_ptr<FStaticMeshResource>   GetResource() const { return Resource; }

  private:
    std::shared_ptr<FStaticMeshCookedData> CookedData;
    std::shared_ptr<FStaticMeshResource>   Resource;
};
