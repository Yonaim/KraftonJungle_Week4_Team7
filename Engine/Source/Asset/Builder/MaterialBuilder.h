#pragma once

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

    std::shared_ptr<FMaterialCookedData> Build(const FWString& Path);

  private:
    std::shared_ptr<FIntermediateMaterialData> ParseMaterial(const FSourceRecord& Source);
    std::shared_ptr<FMaterialCookedData>       CookMaterial(const FSourceRecord&            Source,
                                                            const FIntermediateMaterialData& Intermediate);

    static bool                  ReadAllText(const FWString& Path, FString& OutText);
    static FString               Trim(const FString& Value);
    static EMaterialTextureSlot  ResolveTextureSlot(const FString& SlotName);
    static FWString              ResolveRelativePath(const FWString& BasePath, const FString& RelativePath);

  private:
    FAssetBuildCache& Cache;
};

} // namespace Asset
