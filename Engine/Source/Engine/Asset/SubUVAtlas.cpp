#include "Engine/Asset/SubUVAtlas.h"

#include <filesystem>

REGISTER_CLASS(, USubUVAtlas)

namespace
{
    static FString BuildAssetNameFromPath(const FString& InAssetPath)
    {
        if (InAssetPath.empty())
        {
            return {};
        }

        const std::filesystem::path FilePath(InAssetPath);
        return FilePath.stem().string();
    }
} // namespace

bool USubUVAtlas::LoadFromCooked(const FString&                         InAssetPath,
                                 std::shared_ptr<FSubUVAtlasCookedData> InCookedData,
                                 RHI::FDynamicRHI&                      InDynamicRHI)
{
    if (InCookedData == nullptr)
    {
        return false;
    }

    if (CookedData == InCookedData && GetAssetPath() == InAssetPath && RenderResource != nullptr)
    {
        SetLoaded(true);
        return true;
    }

    std::shared_ptr<FSubUVAtlasRenderResource> NewRenderResource =
        FSubUVAtlasRenderResource::Create(*InCookedData, InDynamicRHI);
    if (NewRenderResource == nullptr)
    {
        return false;
    }

    SetAssetPath(InAssetPath);
    SetAssetName(BuildAssetNameFromPath(InAssetPath));
    SetCookedData(std::move(InCookedData));
    SetRenderResource(std::move(NewRenderResource));
    SetLoaded(true);
    return true;
}
