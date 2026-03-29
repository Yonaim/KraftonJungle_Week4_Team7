#pragma once

#include "CoreUObject/Object.h"

class UAsset : public UObject
{
    DECLARE_RTTI(UAsset, UObject)

  public:
    void           SetAssetPath(const FString& InAssetPath) { AssetPath = InAssetPath; }
    const FString& GetAssetPath() const { return AssetPath; }

  private:
    FString AssetPath;
};
