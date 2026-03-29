#pragma once

#include "Asset/Core/Types.h"
#include "Asset/Data/SubUVAtlasCookedData.h"

struct FIntermediateSubUVAtlasData
{
    FSubUVAtlasInfo               Info;
    TArray<FSubUVFrame>           Frames;
    TMap<FString, FSubUVSequence> Sequences;
    FWString                      AtlasImagePath;
};
