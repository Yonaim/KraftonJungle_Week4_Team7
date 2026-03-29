#pragma once

#include <memory>

#include "Asset/AssetBuildCache.h"

class FMaterialBuilder
{
  public:
    explicit FMaterialBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

  private:
    FAssetBuildCache& Cache;
};
// TODO
