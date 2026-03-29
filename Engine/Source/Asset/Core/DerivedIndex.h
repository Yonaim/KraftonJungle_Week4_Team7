#pragma once

#include "Core/CoreMinimal.h"
#include "Asset/Core/AssetKey.h"

class FDerivedIndex
{
  public:
    const FDerivedKey* Find(const FWString& Path) const;
    const FDerivedKey* Find(const FPathKey& PathKey) const;

    void Set(const FWString& Path, const FDerivedKey& DerivedKey);
    void Set(const FPathKey& PathKey, const FDerivedKey& DerivedKey);

    bool Remove(const FWString& Path);
    bool Remove(const FPathKey& PathKey);

    void Clear();

  private:
    TMap<FPathKey, FDerivedKey> Records;
};
