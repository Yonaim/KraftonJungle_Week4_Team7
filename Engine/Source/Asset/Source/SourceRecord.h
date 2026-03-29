#pragma once

#include "Core/CoreMinimal.h"

struct FSourceRecord
{
    FWString NormalizedPath;
    uint64   FileSize = 0;
    uint64   LastWriteTimeTicks = 0;
    FString  ContentHash;
    bool     bHasContentHash = false;
};
