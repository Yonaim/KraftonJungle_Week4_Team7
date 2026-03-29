#pragma once

#include "Core/CoreMinimal.h"

class FSourceHash
{
  public:
    static bool Compute(const TArray<uint8>& Bytes, FString& OutHash);
};
