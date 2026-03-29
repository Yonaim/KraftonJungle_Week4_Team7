#pragma once

#include <memory>

#include "Asset/Cache/AssetBuildCache.h"
#include "Asset/Cache/BuildSettings.h"
#include "Asset/Cooked/TextureCookedData.h"
#include "Asset/Intermediate/IntermediateTextureData.h"

namespace Asset
{
    class FTextureBuilder
    {
      public:
        explicit FTextureBuilder(FAssetBuildCache& InCache);

        std::shared_ptr<FTextureCookedData> Build(const FWString&              Path,
                                                  const FTextureBuildSettings& Settings = {});

      private:
        bool DecodeTexture(const FSourceRecord& Source, FIntermediateTextureData& OutData) const;

        std::shared_ptr<FTextureCookedData>
        CookTexture(const FSourceRecord& Source, const FIntermediateTextureData& Intermediate,
                    const FTextureBuildSettings& Settings) const;

      private:
        FAssetBuildCache& Cache;
    };
} // namespace Asset
