#include "Asset/Manager/AssetCacheManager.h"

#include <filesystem>

#include "Engine/Scene/SceneAssetPath.h"

namespace Asset
{
    FString FAssetCacheManager::StringFromPath(const std::filesystem::path& Path)
    {
        return Path.empty() ? FString{} : Path.generic_string();
    }

    std::filesystem::path FAssetCacheManager::ResolveAssetPath(const FString& Path)
    {
        if (Path.empty())
        {
            return {};
        }

        std::filesystem::path Candidate(Path);
        if (Candidate.is_absolute())
        {
            return Candidate;
        }

        return Engine::Scene::ResolveSceneAssetPathToAbsolute(Path);
    }

    FAssetCacheManager::FAssetCacheManager()
        : BuildCache(), TextureBuilder(BuildCache), MaterialBuilder(BuildCache),
          StaticMeshBuilder(BuildCache), SubUVAtlasBuilder(BuildCache), FontAtlasBuilder(BuildCache)
    {
    }

    std::shared_ptr<FTextureCookedData>
    FAssetCacheManager::BuildTexture(const FString& Path, const FTextureBuildSettings& Settings)
    {
        const std::filesystem::path AbsolutePath = ResolveAssetPath(Path);
        if (AbsolutePath.empty())
        {
            UE_LOG(FEditor, ELogVerbosity::Error, "Texture asset path resolve failed: %s",
                   Path.c_str());
            return nullptr;
        }

        UE_LOG(FEditor, ELogVerbosity::Log, "Texture asset path resolved: %s -> %s", Path.c_str(),
               StringFromPath(AbsolutePath).c_str());

        return BuildTextureAbsolute(AbsolutePath, Settings);
    }

    std::shared_ptr<FMaterialCookedData> FAssetCacheManager::BuildMaterial(const FString& Path)
    {
        FString LibraryPath = Path;
        FString MaterialName;
        FMaterialBuilder::SplitMaterialAssetPath(Path, LibraryPath, MaterialName);

        const std::filesystem::path AbsolutePath = ResolveAssetPath(LibraryPath);
        if (AbsolutePath.empty())
        {
            UE_LOG(FEditor, ELogVerbosity::Error, "Material asset path resolve failed: %s",
                   Path.c_str());
            return nullptr;
        }

        const FString ResolvedPath = MaterialName.empty()
                                         ? StringFromPath(AbsolutePath)
                                         : FMaterialBuilder::MakeMaterialAssetPath(AbsolutePath, MaterialName);
        UE_LOG(FEditor, ELogVerbosity::Log, "Material asset path resolved: %s -> %s", Path.c_str(),
               ResolvedPath.c_str());

        return BuildMaterialAbsolute(AbsolutePath, MaterialName);
    }

    std::shared_ptr<FStaticMeshCookedData>
    FAssetCacheManager::BuildStaticMesh(const FString&                  Path,
                                        const FStaticMeshBuildSettings& Settings)
    {
        const std::filesystem::path AbsolutePath = ResolveAssetPath(Path);
        if (AbsolutePath.empty())
        {
            UE_LOG(FEditor, ELogVerbosity::Error, "Static mesh asset path resolve failed: %s",
                   Path.c_str());
            return nullptr;
        }

        UE_LOG(FEditor, ELogVerbosity::Log, "Static mesh asset path resolved: %s -> %s",
               Path.c_str(), StringFromPath(AbsolutePath).c_str());

        return BuildStaticMeshAbsolute(AbsolutePath, Settings);
    }

    std::shared_ptr<FSubUVAtlasCookedData>
    FAssetCacheManager::BuildSubUVAtlas(const FString&               Path,
                                        const FTextureBuildSettings& AtlasTextureSettings)
    {
        const std::filesystem::path AbsolutePath = ResolveAssetPath(Path);
        if (AbsolutePath.empty())
        {
            UE_LOG(FEditor, ELogVerbosity::Error, "SubUV atlas asset path resolve failed: %s",
                   Path.c_str());
            return nullptr;
        }

        UE_LOG(FEditor, ELogVerbosity::Log, "SubUV atlas asset path resolved: %s -> %s",
               Path.c_str(), StringFromPath(AbsolutePath).c_str());

        return BuildSubUVAtlasAbsolute(AbsolutePath, AtlasTextureSettings);
    }

    std::shared_ptr<FFontAtlasCookedData>
    FAssetCacheManager::BuildFontAtlas(const FString&               Path,
                                       const FTextureBuildSettings& AtlasTextureSettings)
    {
        const std::filesystem::path AbsolutePath = ResolveAssetPath(Path);
        if (AbsolutePath.empty())
        {
            UE_LOG(FEditor, ELogVerbosity::Error, "Font atlas asset path resolve failed: %s",
                   Path.c_str());
            return nullptr;
        }

        UE_LOG(FEditor, ELogVerbosity::Log, "Font atlas asset path resolved: %s -> %s",
               Path.c_str(), StringFromPath(AbsolutePath).c_str());

        return BuildFontAtlasAbsolute(AbsolutePath, AtlasTextureSettings);
    }

    std::shared_ptr<FTextureCookedData>
    FAssetCacheManager::BuildTextureAbsolute(const std::filesystem::path& AbsolutePath,
                                             const FTextureBuildSettings& Settings)
    {
        std::shared_ptr<FTextureCookedData> Result = TextureBuilder.Build(AbsolutePath, Settings);

        if (Result)
        {
            UE_LOG(FEditor, ELogVerbosity::Log, "Texture asset load succeeded: %s",
                   StringFromPath(AbsolutePath).c_str());
        }
        else
        {
            UE_LOG(FEditor, ELogVerbosity::Error, "Texture asset load failed: %s",
                   StringFromPath(AbsolutePath).c_str());
        }

        return Result;
    }

    std::shared_ptr<FMaterialCookedData>
    FAssetCacheManager::BuildMaterialAbsolute(const std::filesystem::path& AbsolutePath,
                                              const FString&               MaterialName)
    {
        std::shared_ptr<FMaterialCookedData> Result = MaterialBuilder.BuildMaterial(AbsolutePath, MaterialName);

        const FString LogPath = MaterialName.empty()
                                    ? StringFromPath(AbsolutePath)
                                    : FMaterialBuilder::MakeMaterialAssetPath(AbsolutePath, MaterialName);
        if (Result)
        {
            UE_LOG(FEditor, ELogVerbosity::Log, "Material asset load succeeded: %s", LogPath.c_str());
        }
        else
        {
            UE_LOG(FEditor, ELogVerbosity::Error, "Material asset load failed: %s", LogPath.c_str());
        }

        return Result;
    }

    std::shared_ptr<FStaticMeshCookedData>
    FAssetCacheManager::BuildStaticMeshAbsolute(const std::filesystem::path&    AbsolutePath,
                                                const FStaticMeshBuildSettings& Settings)
    {
        std::shared_ptr<FStaticMeshCookedData> Result =
            StaticMeshBuilder.Build(AbsolutePath, Settings);

        if (Result)
        {
            UE_LOG(FEditor, ELogVerbosity::Log, "Static mesh asset load succeeded: %s",
                   StringFromPath(AbsolutePath).c_str());
        }
        else
        {
            UE_LOG(FEditor, ELogVerbosity::Error, "Static mesh asset load failed: %s",
                   StringFromPath(AbsolutePath).c_str());
        }

        return Result;
    }

    std::shared_ptr<FSubUVAtlasCookedData> FAssetCacheManager::BuildSubUVAtlasAbsolute(
        const std::filesystem::path& AbsolutePath, const FTextureBuildSettings& AtlasTextureSettings)
    {
        std::shared_ptr<FSubUVAtlasCookedData> Result =
            SubUVAtlasBuilder.Build(AbsolutePath, AtlasTextureSettings);

        if (Result)
        {
            UE_LOG(FEditor, ELogVerbosity::Log, "SubUV atlas asset load succeeded: %s",
                   StringFromPath(AbsolutePath).c_str());
        }
        else
        {
            UE_LOG(FEditor, ELogVerbosity::Error, "SubUV atlas asset load failed: %s",
                   StringFromPath(AbsolutePath).c_str());
        }

        return Result;
    }

    std::shared_ptr<FFontAtlasCookedData> FAssetCacheManager::BuildFontAtlasAbsolute(
        const std::filesystem::path& AbsolutePath, const FTextureBuildSettings& AtlasTextureSettings)
    {
        std::shared_ptr<FFontAtlasCookedData> Result =
            FontAtlasBuilder.Build(AbsolutePath, AtlasTextureSettings);

        if (Result)
        {
            UE_LOG(FEditor, ELogVerbosity::Log, "Font atlas asset load succeeded: %s",
                   StringFromPath(AbsolutePath).c_str());
        }
        else
        {
            UE_LOG(FEditor, ELogVerbosity::Error, "Font atlas asset load failed: %s",
                   StringFromPath(AbsolutePath).c_str());
        }

        return Result;
    }

    void FAssetCacheManager::ClearAll() { BuildCache.ClearAll(); }

} // namespace Asset
