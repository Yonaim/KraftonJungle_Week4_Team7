#pragma once

#include <memory>

#include "Asset/Data/FontAtlasCookedData.h"
#include "Engine/Asset/Asset.h"
#include "Renderer/RenderAsset/TextureResource.h"

class UFontAtlas : public UAsset
{
    DECLARE_RTTI(UFontAtlas, UAsset)

  public:
    void SetCookedData(const std::shared_ptr<FFontAtlasCookedData>& InCookedData)
    {
        CookedData = InCookedData;
    }
    void SetAtlasResource(const std::shared_ptr<FTextureResource>& InAtlasResource)
    {
        AtlasResource = InAtlasResource;
    }

    std::shared_ptr<FFontAtlasCookedData> GetCookedData() const { return CookedData; }
    const FFontGlyph*                     FindGlyph(uint32 InCodePoint) const
    {
        return CookedData ? CookedData->FindGlyph(InCodePoint) : nullptr;
    }

    ID3D11ShaderResourceView* GetSRV() const
    {
        return AtlasResource ? AtlasResource->GetSRV() : nullptr;
    }

  private:
    std::shared_ptr<FFontAtlasCookedData> CookedData;
    std::shared_ptr<FTextureResource>     AtlasResource;
};
