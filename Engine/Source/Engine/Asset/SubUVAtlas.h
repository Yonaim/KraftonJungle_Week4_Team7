#pragma once

#include <memory>

#include "Asset/Data/SubUVAtlasCookedData.h"
#include "Engine/Asset/Asset.h"
#include "Renderer/Resource/TextureResource.h"

class USubUVAtlas : public UAsset
{
    DECLARE_RTTI(USubUVAtlas, UAsset)

  public:
    void SetCookedData(const std::shared_ptr<FSubUVAtlasCookedData>& InCookedData)
    {
        CookedData = InCookedData;
    }
    void SetAtlasResource(const std::shared_ptr<FTextureResource>& InAtlasResource)
    {
        AtlasResource = InAtlasResource;
    }

    std::shared_ptr<FSubUVAtlasCookedData> GetCookedData() const { return CookedData; }

    const FSubUVFrame* FindFrame(uint32 InId) const
    {
        return CookedData ? CookedData->FindFrame(InId) : nullptr;
    }

    ID3D11ShaderResourceView* GetSRV() const
    {
        return AtlasResource ? AtlasResource->GetSRV() : nullptr;
    }

  private:
    std::shared_ptr<FSubUVAtlasCookedData> CookedData;
    std::shared_ptr<FTextureResource>      AtlasResource;
};
