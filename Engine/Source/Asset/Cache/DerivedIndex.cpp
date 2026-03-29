#include "Asset/Cache/DerivedIndex.h"

#include "Core/Misc/Paths.h"

namespace Asset
{

const FDerivedKey* FDerivedIndex::Find(const FWString& Path) const
{
    const FWString NormalizedPath = FPaths::Normalize(Path);
    if (NormalizedPath.empty())
    {
        return nullptr;
    }

    return Find(FPathKey(NormalizedPath));
}

const FDerivedKey* FDerivedIndex::Find(const FPathKey& PathKey) const
{
    auto It = Records.find(PathKey);
    if (It == Records.end())
    {
        return nullptr;
    }

    return &It->second;
}

void FDerivedIndex::Set(const FWString& Path, const FDerivedKey& DerivedKey)
{
    const FWString NormalizedPath = FPaths::Normalize(Path);
    if (NormalizedPath.empty())
    {
        return;
    }

    Set(FPathKey(NormalizedPath), DerivedKey);
}

void FDerivedIndex::Set(const FPathKey& PathKey, const FDerivedKey& DerivedKey)
{
    if (!PathKey.IsValid())
    {
        return;
    }

    Records[PathKey] = DerivedKey;
}

bool FDerivedIndex::Remove(const FWString& Path)
{
    const FWString NormalizedPath = FPaths::Normalize(Path);
    if (NormalizedPath.empty())
    {
        return false;
    }

    return Remove(FPathKey(NormalizedPath));
}

bool FDerivedIndex::Remove(const FPathKey& PathKey)
{
    auto It = Records.find(PathKey);
    if (It == Records.end())
    {
        return false;
    }

    Records.erase(It);
    return true;
}

void FDerivedIndex::Clear()
{
    Records.clear();
}

} // namespace Asset
