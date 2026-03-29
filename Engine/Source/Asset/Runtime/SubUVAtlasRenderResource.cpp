#include "Asset/Runtime/SubUVAtlasRenderResource.h"

#include "Asset/Runtime/TextureRenderResource.h"

namespace Asset
{
    std::shared_ptr<FSubUVAtlasRenderResource>
    FSubUVAtlasRenderResource::Create(const FSubUVAtlasCookedData& CookedData,
                                      RHI::FDynamicRHI&            RHI)
    {
        if (!CookedData.IsValid() || CookedData.AtlasTexture == nullptr)
        {
            return nullptr;
        }

        std::shared_ptr<FTextureRenderResource> TextureResource =
            FTextureRenderResource::Create(*CookedData.AtlasTexture, RHI);
        if (TextureResource == nullptr || !TextureResource->IsValid())
        {
            return nullptr;
        }

        std::shared_ptr<FSubUVAtlasRenderResource> Resource =
            std::make_shared<FSubUVAtlasRenderResource>();
        Resource->AtlasTexture = std::move(TextureResource->Texture);
        return Resource;
    }

    bool FSubUVAtlasRenderResource::IsValid() const { return AtlasTexture != nullptr; }

    void FSubUVAtlasRenderResource::Reset() { AtlasTexture.reset(); }
} // namespace Asset
