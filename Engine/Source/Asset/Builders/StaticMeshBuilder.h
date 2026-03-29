#pragma once

#include <memory>

#include "Asset/AssetBuildCache.h"
#include "Asset/Core/BuildSettings.h"

namespace Asset
{

class FStaticMeshBuilder
{
  public:
    explicit FStaticMeshBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

  private:
    FAssetBuildCache& Cache;
};
// TODO

} // namespace Asset
