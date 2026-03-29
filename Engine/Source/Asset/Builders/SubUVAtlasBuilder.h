#pragma once

#include <memory>

#include "Asset/AssetBuildCache.h"

namespace Asset
{

class FSubUVAtlasBuilder
{
  public:
    explicit FSubUVAtlasBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

  private:
    FAssetBuildCache& Cache;
};
// TODO

} // namespace Asset
