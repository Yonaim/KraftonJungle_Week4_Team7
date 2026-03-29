#pragma once

#include "Asset/Cache/AssetKey.h"
#include "Asset/Core/AssetDataBase.h"

inline FDerivedKey MakeIntermediateKey(Asset::EAssetType AssetType, const FString& SourceHash)
{
    return FDerivedKey(std::to_string(static_cast<int>(AssetType)) + "|" + SourceHash + "|I");
}

inline FDerivedKey MakeCookedKey(Asset::EAssetType AssetType, const FString& SourceHash,
                                 const FString& BuildSettingsKey)
{
    return FDerivedKey(std::to_string(static_cast<int>(AssetType)) + "|" + SourceHash + "|C|" +
                       BuildSettingsKey);
}

inline FDerivedKey MakeDerivedKey(const FString& SourceHash, const FString& BuildSettingsKey)
{
    return MakeCookedKey(Asset::EAssetType::Unknown, SourceHash, BuildSettingsKey);
}
