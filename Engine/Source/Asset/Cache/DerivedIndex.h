#pragma once

#include "Core/CoreMinimal.h"
#include "Asset/Cache/AssetKey.h"
#include "Asset/Cache/DerivedKey.h"

namespace Asset
{

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

} // namespace Asset
