#include "Asset/Source/SourceCache.h"

#include <filesystem>

#include "Asset/Source/SourceHash.h"
#include "Asset/Source/SourceLoader.h"

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
    } // namespace

    const FSourceRecord* FSourceCache::GetOrLoad(const FWString& Path)
    {
        const FWString NormalizedPath = NormalizePath(Path);
        if (NormalizedPath.empty())
        {
            return nullptr;
        }

        uint64 CurrentFileSize = 0;
        uint64 CurrentWriteTimeTicks = 0;
        if (!FSourceLoader::QueryFileInfo(NormalizedPath, CurrentFileSize, CurrentWriteTimeTicks))
        {
            Records.erase(NormalizedPath);
            return nullptr;
        }

        auto ExistingIt = Records.find(NormalizedPath);
        if (ExistingIt != Records.end())
        {
            if (!HasFileChanged(ExistingIt->second, CurrentFileSize, CurrentWriteTimeTicks))
            {
                return &ExistingIt->second;
            }
        }

        FSourceRecord NewRecord;
        if (!ReloadRecord(NormalizedPath, NewRecord))
        {
            Records.erase(NormalizedPath);
            return nullptr;
        }

        auto [It, bInserted] = Records.insert_or_assign(NormalizedPath, std::move(NewRecord));
        (void)bInserted;
        return &It->second;
    }

    bool FSourceCache::EnsureContentHashLoaded(const FWString& Path)
    {
        const FWString NormalizedPath = NormalizePath(Path);
        if (NormalizedPath.empty())
        {
            return false;
        }

        auto It = Records.find(NormalizedPath);
        if (It == Records.end())
        {
            return false;
        }

        if (It->second.bHasContentHash)
        {
            return true;
        }

        TArray<uint8> FileBytes;
        if (!FSourceLoader::ReadAllBytes(NormalizedPath, FileBytes))
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

    const FSourceRecord* FSourceCache::Find(const FWString& Path) const
    {
        const FWString NormalizedPath = NormalizePath(Path);
        if (NormalizedPath.empty())
        {
            return nullptr;
        }

        auto It = Records.find(NormalizedPath);
        if (It == Records.end())
        {
            return nullptr;
        }

        return &It->second;
    }

    void FSourceCache::Invalidate(const FWString& Path)
    {
        const FWString NormalizedPath = NormalizePath(Path);
        if (!NormalizedPath.empty())
        {
            Records.erase(NormalizedPath);
        }
    }

    void FSourceCache::Clear() { Records.clear(); }

    bool FSourceCache::HasFileChanged(const FSourceRecord& Record, uint64 CurrentFileSize,
                                      uint64 CurrentWriteTimeTicks) const
    {
        return Record.FileSize != CurrentFileSize ||
               Record.LastWriteTimeTicks != CurrentWriteTimeTicks;
    }

    bool FSourceCache::ReloadRecord(const FWString& NormalizedPath, FSourceRecord& OutRecord) const
    {
        uint64 FileSize = 0;
        uint64 WriteTimeTicks = 0;
        if (!FSourceLoader::QueryFileInfo(NormalizedPath, FileSize, WriteTimeTicks))
        {
            return false;
        }

        OutRecord = {};
        OutRecord.NormalizedPath = NormalizedPath;
        OutRecord.FileSize = FileSize;
        OutRecord.LastWriteTimeTicks = WriteTimeTicks;
        return true;
    }

} // namespace Asset
