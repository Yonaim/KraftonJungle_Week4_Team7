#include "Engine/Asset/Material.h"

#include <filesystem>

REGISTER_CLASS(, UMaterial)

namespace
{
    static FString BuildAssetNameFromPath(const FString&                         InAssetPath,
                                          const std::shared_ptr<FMtlCookedData>& InCookedData)
    {
        if (InCookedData != nullptr && !InCookedData->Name.empty())
        {
            return InCookedData->Name;
        }

        if (InAssetPath.empty())
        {
            return {};
        }

        const std::filesystem::path FilePath(InAssetPath);
        return FilePath.stem().string();
    }
} // namespace

bool UMaterial::LoadFromCooked(const FString&                  InAssetPath,
                               std::shared_ptr<FMtlCookedData> InCookedData,
                               RHI::FDynamicRHI&               InDynamicRHI)
{
    if (InCookedData == nullptr || !InCookedData->IsValid())
    {
        return false;
    }

    if (CookedData == InCookedData && GetAssetPath() == InAssetPath && RenderResource != nullptr)
    {
        SetLoaded(true);
        return true;
    }

    std::shared_ptr<FMaterialRenderResource> NewRenderResource =
        FMaterialRenderResource::Create(*InCookedData, InDynamicRHI);
    if (NewRenderResource == nullptr)
    {
        return false;
    }

    SetAssetPath(InAssetPath);
    SetAssetName(BuildAssetNameFromPath(InAssetPath, InCookedData));
    SetCookedData(std::move(InCookedData));
    SetRenderResource(std::move(NewRenderResource));
    SetLoaded(true);
    return true;
}