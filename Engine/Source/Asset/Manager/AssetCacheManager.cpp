#include "Asset/Manager/AssetCacheManager.h"

namespace Asset
{

    FAssetCacheManager::FAssetCacheManager()
        : BuildCache(), TextureBuilder(BuildCache), MaterialBuilder(BuildCache),
          StaticMeshBuilder(BuildCache), SubUVAtlasBuilder(BuildCache), FontAtlasBuilder(BuildCache)
    {
    }

    std::shared_ptr<FTextureCookedData>
    FAssetCacheManager::BuildTexture(const std::filesystem::path& Path,
                                     const FTextureBuildSettings& Settings)
    {
        return TextureBuilder.Build(Path, Settings);
    }

    std::shared_ptr<FMaterialCookedData>
    FAssetCacheManager::BuildMaterial(const std::filesystem::path& Path)
    {
        return MaterialBuilder.Build(Path);
    }

    std::shared_ptr<FStaticMeshCookedData>
    FAssetCacheManager::BuildStaticMesh(const std::filesystem::path&    Path,
                                        const FStaticMeshBuildSettings& Settings)
    {
        return StaticMeshBuilder.Build(Path, Settings);
    }

    std::shared_ptr<FSubUVAtlasCookedData>
    FAssetCacheManager::BuildSubUVAtlas(const std::filesystem::path& Path,
                                        const FTextureBuildSettings& AtlasTextureSettings)
    {
        return SubUVAtlasBuilder.Build(Path, AtlasTextureSettings);
    }

    std::shared_ptr<FFontAtlasCookedData>
    FAssetCacheManager::BuildFontAtlas(const std::filesystem::path& Path,
                                       const FTextureBuildSettings& AtlasTextureSettings)
    {
        return FontAtlasBuilder.Build(Path, AtlasTextureSettings);
    }

    void FAssetCacheManager::ClearAll() { BuildCache.ClearAll(); }

} // namespace Asset
