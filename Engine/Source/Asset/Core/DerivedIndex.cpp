#include "Asset/Core/DerivedIndex.h"
#include "Core/Misc/Paths.h"

namespace
{
    static FPathKey MakePathKey(const FWString& Path)
    {
        const FWString NormalizedPath = FPaths::Normalize(Path);
        if (NormalizedPath.empty())
        {
            return FPathKey{};
        }

        return FPathKey(NormalizedPath);
    }
}

// ======================== Find ==========================

const FDerivedKey* FDerivedIndex::Find(const FWString& Path) const
{
    return Find(MakePathKey(Path));
}

const FDerivedKey* FDerivedIndex::Find(const FPathKey& PathKey) const
{
    if (!PathKey.IsValid())
    {
        return nullptr;
    }

    auto It = Records.find(PathKey);
    if (It == Records.end())
    {
        return nullptr;
    }

    return &It->second;
}

// ======================== Set ==========================

void FDerivedIndex::Set(const FWString& Path, const FDerivedKey& DerivedKey)
{
    Set(MakePathKey(Path), DerivedKey);
}

void FDerivedIndex::Set(const FPathKey& PathKey, const FDerivedKey& DerivedKey)
{
    if (!PathKey.IsValid() || !DerivedKey.IsValid())
    {
        return;
    }

    Records[PathKey] = DerivedKey;
}

// ======================== Remove ==========================

bool FDerivedIndex::Remove(const FWString& Path)
{
    return Remove(MakePathKey(Path));
}

bool FDerivedIndex::Remove(const FPathKey& PathKey)
{
    if (!PathKey.IsValid())
    {
        return false;
    }

    auto It = Records.find(PathKey);
    if (It == Records.end())
    {
        return false;
    }

    Records.erase(It);
    return true;
}

// ======================== Clear ==========================

void FDerivedIndex::Clear() { Records.clear(); }
