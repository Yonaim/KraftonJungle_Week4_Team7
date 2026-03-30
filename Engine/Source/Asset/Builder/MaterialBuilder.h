#pragma once

#include <filesystem>
#include <memory>

#include "Asset/Cache/AssetBuildCache.h"
#include "Asset/Builder/TextureBuilder.h"
#include "Asset/Cooked/MaterialCookedData.h"
#include "Asset/Intermediate/IntermediateMaterialData.h"

namespace Asset
{

class FMaterialBuilder
{
  public:
    explicit FMaterialBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

    std::shared_ptr<FMaterialCookedLibraryData> BuildLibrary(const std::filesystem::path& Path);
    std::shared_ptr<FMaterialCookedLibraryData> BuildLibrary(const FWString& Path)
    {
        return BuildLibrary(std::filesystem::path(Path));
    }

    std::shared_ptr<FMaterialCookedData> BuildMaterial(const std::filesystem::path& Path,
                                                       const FString& MaterialName = {});
    std::shared_ptr<FMaterialCookedData> BuildMaterial(const FWString& Path,
                                                       const FString& MaterialName = {})
    {
        return BuildMaterial(std::filesystem::path(Path), MaterialName);
    }

    static FString MakeMaterialAssetPath(const std::filesystem::path& LibraryPath,
                                         const FString&               MaterialName);
    static bool    SplitMaterialAssetPath(const FString& InAssetPath, FString& OutLibraryPath,
                                          FString& OutMaterialName);

  private:
    std::shared_ptr<FIntermediateMaterialLibraryData> ParseMaterialLibrary(const FSourceRecord& Source);
    std::shared_ptr<FMaterialCookedLibraryData> CookMaterialLibrary(
        const FSourceRecord& Source, const FIntermediateMaterialLibraryData& Intermediate);

    static bool                 ReadAllText(const std::filesystem::path& Path, FString& OutText);
    static FString              Trim(const FString& Value);
    static EMaterialTextureSlot ResolveTextureSlot(const FString& SlotName);
    static FWString ResolveRelativePath(const std::filesystem::path& BasePath,
                                        const FString&               RelativePath);

  private:
    FAssetBuildCache& Cache;
};

} // namespace Asset
