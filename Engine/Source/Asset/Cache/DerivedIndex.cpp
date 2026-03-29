#include "Asset/Cache/DerivedIndex.h"

#include <filesystem>

namespace Asset
{

namespace
{
    static FWString NormalizePath(const FWString& InPath)
    {
        if (InPath.empty())
        {
            return {};
        }

        std::error_code Ec;
        std::filesystem::path P(InPath);
        P = std::filesystem::weakly_canonical(P, Ec);
        if (Ec)
        {
            Ec.clear();
            P = std::filesystem::absolute(P, Ec);
            if (Ec)
            {
                return {};
            }
        }

        return P.native();
    }
}

const FDerivedKey* FDerivedIndex::Find(const FWString& Path) const
{
    const FWString NormalizedPath = NormalizePath(Path);
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
    const FWString NormalizedPath = NormalizePath(Path);
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
    const FWString NormalizedPath = NormalizePath(Path);
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
