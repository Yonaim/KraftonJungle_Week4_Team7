#pragma once

#include <filesystem>
#include <memory>

#include "Asset/Cache/AssetBuildCache.h"
#include "Asset/Cache/BuildSettings.h"
#include "Asset/Cooked/StaticMeshCookedData.h"
#include "Asset/Intermediate/IntermediateStaticMeshData.h"

namespace Asset
{

class FStaticMeshBuilder
{
  public:
    explicit FStaticMeshBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

    std::shared_ptr<FStaticMeshCookedData> Build(const std::filesystem::path& Path,
                                                 const FStaticMeshBuildSettings& Settings = {});
    std::shared_ptr<FStaticMeshCookedData> Build(const FWString& Path,
                                                 const FStaticMeshBuildSettings& Settings = {})
    {
        return Build(std::filesystem::path(Path), Settings);
    }

  private:
    std::shared_ptr<FIntermediateStaticMeshData> ParseObj(const FSourceRecord& Source);
    std::shared_ptr<FStaticMeshCookedData>       CookMesh(const FSourceRecord&               Source,
                                                          const FIntermediateStaticMeshData& Intermediate,
                                                          const FStaticMeshBuildSettings&    Settings);

  private:
    FAssetBuildCache& Cache;
};

} // namespace Asset
