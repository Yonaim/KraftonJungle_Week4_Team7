#pragma once

#include <filesystem>
#include <memory>

#include "Asset/Builder/FontAtlasBuilder.h"
#include "Asset/Builder/MaterialBuilder.h"
#include "Asset/Builder/StaticMeshBuilder.h"
#include "Asset/Builder/SubUVAtlasBuilder.h"
#include "Asset/Builder/TextureBuilder.h"
#include "Asset/Cache/AssetBuildCache.h"
#include "Asset/Cache/BuildSettings.h"

namespace Asset
{

    class FAssetCacheManager
    {
      public:
        FAssetCacheManager();

        std::shared_ptr<FTextureCookedData>
        BuildTexture(const std::filesystem::path& Path, const FTextureBuildSettings& Settings = {});
        std::shared_ptr<FTextureCookedData> BuildTexture(const FWString&              Path,
                                                         const FTextureBuildSettings& Settings = {})
        {
            return BuildTexture(std::filesystem::path(Path), Settings);
        }

        std::shared_ptr<FMaterialCookedData> BuildMaterial(const std::filesystem::path& Path);
        std::shared_ptr<FMaterialCookedData> BuildMaterial(const FWString& Path)
        {
            return BuildMaterial(std::filesystem::path(Path));
        }

        std::shared_ptr<FStaticMeshCookedData>
        BuildStaticMesh(const std::filesystem::path&    Path,
                        const FStaticMeshBuildSettings& Settings = {});
        std::shared_ptr<FStaticMeshCookedData>
        BuildStaticMesh(const FWString& Path, const FStaticMeshBuildSettings& Settings = {})
        {
            return BuildStaticMesh(std::filesystem::path(Path), Settings);
        }

        std::shared_ptr<FSubUVAtlasCookedData>
        BuildSubUVAtlas(const std::filesystem::path& Path,
                        const FTextureBuildSettings& AtlasTextureSettings = {});
        std::shared_ptr<FSubUVAtlasCookedData>
        BuildSubUVAtlas(const FWString&              Path,
                        const FTextureBuildSettings& AtlasTextureSettings = {})
        {
            return BuildSubUVAtlas(std::filesystem::path(Path), AtlasTextureSettings);
        }

        std::shared_ptr<FFontAtlasCookedData>
        BuildFontAtlas(const std::filesystem::path& Path,
                       const FTextureBuildSettings& AtlasTextureSettings = {});
        std::shared_ptr<FFontAtlasCookedData>
        BuildFontAtlas(const FWString& Path, const FTextureBuildSettings& AtlasTextureSettings = {})
        {
            return BuildFontAtlas(std::filesystem::path(Path), AtlasTextureSettings);
        }

        template <typename TTag>
        const FSourceRecord* GetSource(TTag Tag, const std::filesystem::path& Path)
        {
            return BuildCache.GetSource(Tag, Path);
        }

        template <typename TTag> const FSourceRecord* GetSource(TTag Tag, const FWString& Path)
        {
            return BuildCache.GetSource(Tag, Path);
        }

        template <typename TTag> void InvalidateSource(TTag Tag, const std::filesystem::path& Path)
        {
            BuildCache.InvalidateSource(Tag, Path);
        }

        template <typename TTag> void InvalidateSource(TTag Tag, const FWString& Path)
        {
            BuildCache.InvalidateSource(Tag, Path);
        }

        void ClearAll();

        FAssetBuildCache&       GetBuildCache() { return BuildCache; }
        const FAssetBuildCache& GetBuildCache() const { return BuildCache; }

      private:
        FAssetBuildCache   BuildCache;
        FTextureBuilder    TextureBuilder;
        FMaterialBuilder   MaterialBuilder;
        FStaticMeshBuilder StaticMeshBuilder;
        FSubUVAtlasBuilder SubUVAtlasBuilder;
        FFontAtlasBuilder  FontAtlasBuilder;
    };

} // namespace Asset
