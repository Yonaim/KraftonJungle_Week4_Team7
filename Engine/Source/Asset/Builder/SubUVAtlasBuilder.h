#pragma once

#include <memory>

#include "Asset/Cache/AssetBuildCache.h"
#include "Asset/Builder/TextureBuilder.h"
#include "Asset/Cache/BuildSettings.h"
#include "Asset/Cooked/SubUVAtlasCookedData.h"
#include "Asset/Intermediate/IntermediateSubUVAtlasData.h"

namespace Asset
{

class FSubUVAtlasBuilder
{
  public:
    explicit FSubUVAtlasBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

    std::shared_ptr<FSubUVAtlasCookedData> Build(const FWString& Path,
                                                 const FTextureBuildSettings& AtlasTextureSettings = {});

  private:
    std::shared_ptr<FIntermediateSubUVAtlasData> ParseAtlas(const FSourceRecord& Source);
    std::shared_ptr<FSubUVAtlasCookedData>       CookAtlas(const FSourceRecord&             Source,
                                                           const FIntermediateSubUVAtlasData& Intermediate,
                                                           const FTextureBuildSettings&      AtlasTextureSettings);

    static bool     ReadAllText(const FWString& Path, FString& OutText);
    static FString  Trim(const FString& Value);
    static FWString ResolveRelativePath(const FWString& BasePath, const FString& RelativePath);

  private:
    FAssetBuildCache& Cache;
};

} // namespace Asset
