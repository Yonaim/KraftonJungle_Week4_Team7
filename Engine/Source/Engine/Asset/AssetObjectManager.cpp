#include "Engine/Asset/AssetObjectManager.h"

#include <filesystem>

#include "Asset/Builder/MaterialBuilder.h"
#include "Asset/Core/AssetNaming.h"
#include "Asset/Manager/AssetCacheManager.h"
#include "Core/HAL/PlatformTime.h"
#include "Core/Logging/LogMacros.h"
#include "CoreUObject/ObjectIterator.h"
#include "Engine/EngineStatics.h"
#include "Engine/Asset/FontAtlas.h"
#include "Engine/Asset/Material.h"
#include "Engine/Asset/StaticMesh.h"
#include "Engine/Asset/SubUVAtlas.h"
#include "Engine/Asset/Texture.h"

namespace
{
    template <typename TObjectType> TObjectType* FindAssetObjectByPath(const FString& AssetPath)
    {
        for (TObjectIterator<TObjectType> It; It; ++It)
        {
            TObjectType* Object = *It;
            if (Object == nullptr || !Object->IsValidLowLevel())
            {
                continue;
            }

            if (Object->GetAssetPath() == AssetPath)
            {
                return Object;
            }
        }

        return nullptr;
    }
}

namespace
{
    double ToMilliseconds(double Seconds)
    {
        return Seconds * 1000.0;
    }

    template <typename TObjectType>
    TObjectType* LogAssetLoadResult(const char* AssetTypeLabel, const FString& AssetPath,
                                    double StartSeconds, TObjectType* Result,
                                    bool bWasCacheHit = false)
    {
        const double ElapsedMs = ToMilliseconds(FPlatformTime::Seconds() - StartSeconds);
        UEngineStatics::RecordResourceLoad(AssetTypeLabel, AssetPath, ElapsedMs, bWasCacheHit,
                                           Result != nullptr);
        if (Result != nullptr)
        {
            UE_LOG(AssetObject, ELogLevel::Info, "%s load succeeded: %s (%.3f ms%s)",
                   AssetTypeLabel, AssetPath.c_str(), ElapsedMs,
                   bWasCacheHit ? ", cache hit" : "");
        }
        else
        {
            UE_LOG(AssetObject, ELogLevel::Error, "%s load failed: %s (%.3f ms)", AssetTypeLabel,
                   AssetPath.c_str(), ElapsedMs);
        }

        return Result;
    }
}

UObject* FAssetObjectManager::LoadAssetObject(const FString& AssetPath)
{
    if (!IsReady())
    {
        UE_LOG(AssetObject, ELogLevel::Error,
               "AssetObjectManager is not ready to load asset object: %s", AssetPath.c_str());
        return nullptr;
    }

    switch (Asset::ClassifyAssetPath(AssetPath))
    {
    case Asset::EAssetFileKind::StaticMesh:
        return LoadStaticMeshObject(AssetPath);
    case Asset::EAssetFileKind::Texture:
        return LoadTextureObject(AssetPath);
    case Asset::EAssetFileKind::MaterialLibrary:
        return LoadMaterialObject(AssetPath);
    case Asset::EAssetFileKind::TextureAtlas:
        return LoadSubUVAtlasObject(AssetPath);
    case Asset::EAssetFileKind::Font:
        return LoadFontAtlasObject(AssetPath);
    default:
        return nullptr;
    }
}

UStaticMesh* FAssetObjectManager::LoadStaticMeshObject(const FString& AssetPath)
{
    const double StartSeconds = FPlatformTime::Seconds();

    if (!IsReady() || AssetPath.empty())
    {
        return LogAssetLoadResult<UStaticMesh>("Static mesh asset", AssetPath, StartSeconds, nullptr);
    }

    if (UStaticMesh* ExistingObject = FindAssetObjectByPath<UStaticMesh>(AssetPath))
    {
        return LogAssetLoadResult<UStaticMesh>("Static mesh asset", AssetPath, StartSeconds,
                                               ExistingObject, true);
    }

    std::shared_ptr<Asset::FObjCookedData> CookedData = AssetCacheManager->BuildStaticMesh(AssetPath);
    if (CookedData == nullptr)
    {
        return LogAssetLoadResult<UStaticMesh>("Static mesh asset", AssetPath, StartSeconds, nullptr);
    }

    UStaticMesh* NewObject = new UStaticMesh();
    if (!NewObject->LoadFromCooked(AssetPath, std::move(CookedData), *DynamicRHI))
    {
        delete NewObject;
        return LogAssetLoadResult<UStaticMesh>("Static mesh asset", AssetPath, StartSeconds, nullptr);
    }

    BindStaticMeshMaterialSlots(NewObject);
    return LogAssetLoadResult<UStaticMesh>("Static mesh asset", AssetPath, StartSeconds, NewObject);
}

UTexture* FAssetObjectManager::LoadTextureObject(const FString& AssetPath)
{
    const double StartSeconds = FPlatformTime::Seconds();

    if (!IsReady() || AssetPath.empty())
    {
        return LogAssetLoadResult<UTexture>("Texture asset", AssetPath, StartSeconds, nullptr);
    }

    if (UTexture* ExistingObject = FindAssetObjectByPath<UTexture>(AssetPath))
    {
        return LogAssetLoadResult<UTexture>("Texture asset", AssetPath, StartSeconds,
                                            ExistingObject, true);
    }

    std::shared_ptr<Asset::FTextureCookedData> CookedData = AssetCacheManager->BuildTexture(AssetPath);
    if (CookedData == nullptr)
    {
        return LogAssetLoadResult<UTexture>("Texture asset", AssetPath, StartSeconds, nullptr);
    }

    UTexture* NewObject = new UTexture();
    if (!NewObject->LoadFromCooked(AssetPath, std::move(CookedData), *DynamicRHI))
    {
        delete NewObject;
        return LogAssetLoadResult<UTexture>("Texture asset", AssetPath, StartSeconds, nullptr);
    }

    return LogAssetLoadResult<UTexture>("Texture asset", AssetPath, StartSeconds, NewObject);
}

UMaterial* FAssetObjectManager::LoadMaterialObject(const FString& AssetPath)
{
    const double StartSeconds = FPlatformTime::Seconds();

    if (!IsReady() || AssetPath.empty())
    {
        return LogAssetLoadResult<UMaterial>("Material asset", AssetPath, StartSeconds, nullptr);
    }

    if (UMaterial* ExistingObject = FindAssetObjectByPath<UMaterial>(AssetPath))
    {
        return LogAssetLoadResult<UMaterial>("Material asset", AssetPath, StartSeconds,
                                             ExistingObject, true);
    }

    std::shared_ptr<Asset::FMtlCookedData> CookedData = AssetCacheManager->BuildMaterial(AssetPath);
    if (CookedData == nullptr)
    {
        return LogAssetLoadResult<UMaterial>("Material asset", AssetPath, StartSeconds, nullptr);
    }

    UMaterial* NewObject = new UMaterial();
    if (!NewObject->LoadFromCooked(AssetPath, std::move(CookedData), *DynamicRHI))
    {
        delete NewObject;
        return LogAssetLoadResult<UMaterial>("Material asset", AssetPath, StartSeconds, nullptr);
    }

    return LogAssetLoadResult<UMaterial>("Material asset", AssetPath, StartSeconds, NewObject);
}

USubUVAtlas* FAssetObjectManager::LoadSubUVAtlasObject(const FString& AssetPath)
{
    const double StartSeconds = FPlatformTime::Seconds();

    if (!IsReady() || AssetPath.empty())
    {
        return LogAssetLoadResult<USubUVAtlas>("SubUV atlas asset", AssetPath, StartSeconds, nullptr);
    }

    if (USubUVAtlas* ExistingObject = FindAssetObjectByPath<USubUVAtlas>(AssetPath))
    {
        return LogAssetLoadResult<USubUVAtlas>("SubUV atlas asset", AssetPath, StartSeconds,
                                               ExistingObject, true);
    }

    std::shared_ptr<Asset::FSubUVAtlasCookedData> CookedData =
        AssetCacheManager->BuildSubUVAtlas(AssetPath);
    if (CookedData == nullptr)
    {
        return LogAssetLoadResult<USubUVAtlas>("SubUV atlas asset", AssetPath, StartSeconds, nullptr);
    }

    USubUVAtlas* NewObject = new USubUVAtlas();
    if (!NewObject->LoadFromCooked(AssetPath, std::move(CookedData), *DynamicRHI))
    {
        delete NewObject;
        return LogAssetLoadResult<USubUVAtlas>("SubUV atlas asset", AssetPath, StartSeconds, nullptr);
    }

    return LogAssetLoadResult<USubUVAtlas>("SubUV atlas asset", AssetPath, StartSeconds, NewObject);
}

UFontAtlas* FAssetObjectManager::LoadFontAtlasObject(const FString& AssetPath)
{
    const double StartSeconds = FPlatformTime::Seconds();

    if (!IsReady() || AssetPath.empty())
    {
        return LogAssetLoadResult<UFontAtlas>("Font atlas asset", AssetPath, StartSeconds, nullptr);
    }

    if (UFontAtlas* ExistingObject = FindAssetObjectByPath<UFontAtlas>(AssetPath))
    {
        return LogAssetLoadResult<UFontAtlas>("Font atlas asset", AssetPath, StartSeconds,
                                              ExistingObject, true);
    }

    std::shared_ptr<Asset::FFontAtlasCookedData> CookedData = AssetCacheManager->BuildFontAtlas(AssetPath);
    if (CookedData == nullptr)
    {
        return LogAssetLoadResult<UFontAtlas>("Font atlas asset", AssetPath, StartSeconds, nullptr);
    }

    UFontAtlas* NewObject = new UFontAtlas();
    if (!NewObject->LoadFromCooked(AssetPath, std::move(CookedData), *DynamicRHI))
    {
        delete NewObject;
        return LogAssetLoadResult<UFontAtlas>("Font atlas asset", AssetPath, StartSeconds, nullptr);
    }

    return LogAssetLoadResult<UFontAtlas>("Font atlas asset", AssetPath, StartSeconds, NewObject);
}

void FAssetObjectManager::BindStaticMeshMaterialSlots(UStaticMesh* StaticMeshAsset)
{
    if (!IsReady() || StaticMeshAsset == nullptr)
    {
        return;
    }

    const std::shared_ptr<Asset::FObjCookedData>& MeshCookedData = StaticMeshAsset->GetCookedData();
    if (MeshCookedData == nullptr)
    {
        return;
    }

    TArray<UMaterial*>& MaterialSlots = StaticMeshAsset->GetMaterialSlots();
    const size_t        SlotCount = MeshCookedData->Materials.size();

    if (MaterialSlots.size() < SlotCount)
    {
        MaterialSlots.resize(SlotCount, nullptr);
    }

    for (size_t SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
    {
        if (SlotIndex >= MeshCookedData->Materials.size())
        {
            MaterialSlots[SlotIndex] = nullptr;
            continue;
        }

        const Asset::FObjCookedMaterialRef& MaterialRef = MeshCookedData->Materials[SlotIndex];
        if (MaterialRef.Name.empty())
        {
            MaterialSlots[SlotIndex] = nullptr;
            continue;
        }

        if (MaterialRef.LibraryIndex >= MeshCookedData->MaterialLibraries.size())
        {
            MaterialSlots[SlotIndex] = nullptr;
            continue;
        }

        const std::filesystem::path& LibraryPath =
            MeshCookedData->MaterialLibraries[MaterialRef.LibraryIndex];
        if (LibraryPath.empty())
        {
            MaterialSlots[SlotIndex] = nullptr;
            continue;
        }

        const FString MaterialAssetPath =
            Asset::FMaterialBuilder::MakeMaterialAssetPath(LibraryPath, MaterialRef.Name);

        UMaterial* MaterialAsset = LoadMaterialObject(MaterialAssetPath);
        if (MaterialAsset == nullptr)
        {
            UE_LOG(AssetObject, ELogLevel::Error,
                   "Failed to load material object for mesh slot %zu: %s -> %s", SlotIndex,
                   StaticMeshAsset->GetAssetPath().c_str(), MaterialAssetPath.c_str());
            MaterialSlots[SlotIndex] = nullptr;
            continue;
        }

        MaterialSlots[SlotIndex] = MaterialAsset;
        UE_LOG(AssetObject, ELogLevel::Verbose, "Bound material slot %zu: %s -> %s", SlotIndex,
               StaticMeshAsset->GetAssetPath().c_str(), MaterialAssetPath.c_str());
    }
}
