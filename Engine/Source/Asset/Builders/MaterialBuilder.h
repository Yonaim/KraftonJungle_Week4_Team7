#pragma once

#include <memory>

#include "Asset/AssetBuildCache.h"

namespace Asset
{

class FMaterialBuilder
{
  public:
    explicit FMaterialBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

  private:
    FAssetBuildCache& Cache;
};
// TODO

} // namespace Asset
