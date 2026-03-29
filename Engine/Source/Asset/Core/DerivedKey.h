#pragma once

#include "Asset/Core/AssetKey.h"

inline FDerivedKey MakeDerivedKey(const FString& SourceHash, const FString& BuildSettingsKey)
{
    return FDerivedKey(SourceHash + "|" + BuildSettingsKey);
}
