#pragma once

#include <memory>

#include "Asset/AssetBuildCache.h"
#include "Asset/Core/BuildSettings.h"
#include "Asset/Core/DerivedKey.h"

class FTextureBuilder
{
  public:
    explicit FTextureBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

    std::shared_ptr<FTextureCookedData> Build(const FWString&              Path,
                                              const FTextureBuildSettings& Settings);

  private:
    std::shared_ptr<FIntermediateTextureData> DecodeTexture(const FSourceRecord& Source);
    std::shared_ptr<FTextureCookedData>       CookTexture(const FSourceRecord&            Source,
                                                          const FIntermediateTextureData& Intermediate,
                                                          const FTextureBuildSettings&    Settings);

  private:
    FAssetBuildCache& Cache;
};
