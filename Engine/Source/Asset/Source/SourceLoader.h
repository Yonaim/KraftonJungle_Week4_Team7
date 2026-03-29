#pragma once

#include "Core/CoreMinimal.h"

namespace Asset
{

    // FSourceCache에서 내부적으로 사용하는 로더
    class FSourceLoader
    {
      public:
        static bool QueryFileInfo(const FWString& Path, uint64& OutFileSize,
                                  uint64& OutWriteTimeTicks);
        static bool ReadAllBytes(const FWString& Path, TArray<uint8>& OutBytes);
    };

} // namespace Asset
