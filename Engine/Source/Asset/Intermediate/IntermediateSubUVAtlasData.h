#pragma once

#include "Asset/Core/SubUVTypes.h"

namespace Asset
{

    struct FIntermediateSubUVAtlasData
    {
        FSubUVAtlasInfo               Info;
        TArray<FSubUVFrame>           Frames;
        TMap<FString, FSubUVSequence> Sequences;
        FWString                      AtlasImagePath;
    };

} // namespace Asset
