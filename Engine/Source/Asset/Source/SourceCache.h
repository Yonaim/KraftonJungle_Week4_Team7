#pragma once

#include "Asset/Source/SourceRecord.h"

namespace Asset
{

// Source 캐싱 본체
class FSourceCache
{
  public:
    const FSourceRecord* GetOrLoad(const FWString& Path);

    bool EnsureContentHashLoaded(const FWString& Path);

    const FSourceRecord* Find(const FWString& Path) const;

    void Invalidate(const FWString& Path);
    void Clear();

  private:
    bool HasFileChanged(const FSourceRecord& Record, uint64 CurrentFileSize,
                        uint64 CurrentWriteTimeTicks) const;
    bool ReloadRecord(const FWString& NormalizedPath, FSourceRecord& OutRecord) const;

  private:
    TMap<FWString, FSourceRecord> Records; // <NormalizedPath, SourceRecord>
};

} // namespace Asset
