#pragma once

#include <memory>

#include "Asset/Data/TextureCookedData.h"
#include "Engine/Asset/Asset.h"
#include "Renderer/Resource/TextureResource.h"

class UTexture2D : public UAsset
{
    DECLARE_RTTI(UTexture2D, UAsset)

  public:
    void SetCookedData(const std::shared_ptr<FTextureCookedData>& InCookedData)
    {
        CookedData = InCookedData;
    }
    void SetResource(const std::shared_ptr<FTextureResource>& InResource) { Resource = InResource; }

    std::shared_ptr<FTextureCookedData> GetCookedData() const { return CookedData; }
    std::shared_ptr<FTextureResource>   GetResource() const { return Resource; }

    ID3D11ShaderResourceView* GetSRV() const { return Resource ? Resource->GetSRV() : nullptr; }

  private:
    std::shared_ptr<FTextureCookedData> CookedData;
    std::shared_ptr<FTextureResource>   Resource;
};
