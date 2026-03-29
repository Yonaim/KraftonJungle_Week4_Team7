#pragma once

#include <memory>

#include "Asset/AssetBuildCache.h"

namespace Asset
{

class FFontAtlasBuilder
{
  public:
    explicit FFontAtlasBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

  private:
    FAssetBuildCache& Cache;
};
// TODO

} // namespace Asset
