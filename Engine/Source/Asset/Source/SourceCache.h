#pragma once

#include <filesystem>

#include "Asset/Cache/AssetKey.h"
#include "Asset/Source/SourceHash.h"
#include "Asset/Source/SourceLoader.h"
#include "Asset/Source/SourceRecord.h"
#include "Core/Misc/Paths.h"

namespace Asset
{
namespace SourceCacheDetail
{
    inline std::filesystem::path NormalizePath(const std::filesystem::path& InPath)
    {
        if (InPath.empty())
        {
            return {};
        }

        return FPaths::Normalize(InPath);
    }

    template <typename TKey> inline TKey MakeKeyFromNormalizedPath(const std::filesystem::path& NormalizedPath)
    {
        return TKey::FromPath(NormalizedPath);
    }
}

template <typename TSourceKey> class TSourceCache
{
  public:
    const FSourceRecord* GetOrLoad(const std::filesystem::path& Path)
    {
        const std::filesystem::path NormalizedPath = SourceCacheDetail::NormalizePath(Path);
        if (NormalizedPath.empty())
        {
            return nullptr;
        }

        const TSourceKey Key = SourceCacheDetail::MakeKeyFromNormalizedPath<TSourceKey>(NormalizedPath);
        return GetOrLoad(Key);
    }

    const FSourceRecord* GetOrLoad(const TSourceKey& Key)
    {
        if (Key.NormalizedPath.empty())
        {
            return nullptr;
        }

        uint64 CurrentFileSize = 0;
        uint64 CurrentWriteTimeTicks = 0;
        if (!FSourceLoader::QueryFileInfo(Key.NormalizedPath, CurrentFileSize, CurrentWriteTimeTicks))
        {
            Records.erase(Key);
            return nullptr;
        }

        auto ExistingIt = Records.find(Key);
        if (ExistingIt != Records.end())
        {
            if (!HasFileChanged(ExistingIt->second, CurrentFileSize, CurrentWriteTimeTicks))
            {
                return &ExistingIt->second;
            }
        }

        FSourceRecord NewRecord;
        if (!ReloadRecord(Key, NewRecord))
        {
            Records.erase(Key);
            return nullptr;
        }

        auto [It, bInserted] = Records.insert_or_assign(Key, std::move(NewRecord));
        (void)bInserted;
        return &It->second;
    }

    bool EnsureContentHashLoaded(const std::filesystem::path& Path)
    {
        const std::filesystem::path NormalizedPath = SourceCacheDetail::NormalizePath(Path);
        if (NormalizedPath.empty())
        {
            return false;
        }

        const TSourceKey Key = SourceCacheDetail::MakeKeyFromNormalizedPath<TSourceKey>(NormalizedPath);
        return EnsureContentHashLoaded(Key);
    }

    bool EnsureContentHashLoaded(const TSourceKey& Key)
    {
        auto It = Records.find(Key);
        if (It == Records.end())
        {
            return false;
        }
        if (It->second.bHasContentHash)
        {
            return true;
        }

        TArray<uint8> FileBytes;
        if (!FSourceLoader::ReadAllBytes(Key.NormalizedPath, FileBytes))
        {
            return false;
        }

        FString ContentHash;
        if (!FSourceHash::Compute(FileBytes, ContentHash))
        {
            return false;
        }

        It->second.ContentHash = std::move(ContentHash);
        It->second.bHasContentHash = true;
        return true;
    }

    const FSourceRecord* Find(const std::filesystem::path& Path) const
    {
        const std::filesystem::path NormalizedPath = SourceCacheDetail::NormalizePath(Path);
        if (NormalizedPath.empty())
        {
            return nullptr;
        }

        const TSourceKey Key = SourceCacheDetail::MakeKeyFromNormalizedPath<TSourceKey>(NormalizedPath);
        return Find(Key);
    }

    const FSourceRecord* Find(const TSourceKey& Key) const
    {
        auto It = Records.find(Key);
        return It != Records.end() ? &It->second : nullptr;
    }

    void Invalidate(const std::filesystem::path& Path)
    {
        const std::filesystem::path NormalizedPath = SourceCacheDetail::NormalizePath(Path);
        if (NormalizedPath.empty())
        {
            return;
        }
        Records.erase(SourceCacheDetail::MakeKeyFromNormalizedPath<TSourceKey>(NormalizedPath));
    }

    void Invalidate(const TSourceKey& Key) { Records.erase(Key); }
    void Clear() { Records.clear(); }

  private:
    bool HasFileChanged(const FSourceRecord& Record, uint64 CurrentFileSize,
                        uint64 CurrentWriteTimeTicks) const
    {
        return Record.FileSize != CurrentFileSize ||
               Record.LastWriteTimeTicks != CurrentWriteTimeTicks;
    }

    bool ReloadRecord(const TSourceKey& Key, FSourceRecord& OutRecord) const
    {
        uint64 FileSize = 0;
        uint64 WriteTimeTicks = 0;
        if (!FSourceLoader::QueryFileInfo(Key.NormalizedPath, FileSize, WriteTimeTicks))
        {
            return false;
        }

        OutRecord = {};
        OutRecord.NormalizedPath = Key.NormalizedPath;
        OutRecord.FileSize = FileSize;
        OutRecord.LastWriteTimeTicks = WriteTimeTicks;
        return true;
    }

  private:
    TMap<TSourceKey, FSourceRecord> Records;
};

} // namespace Asset
