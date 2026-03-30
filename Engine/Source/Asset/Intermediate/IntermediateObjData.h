#pragma once

#include "Core/CoreMinimal.h"

namespace Asset
{

    struct FIntermediateObjFaceVertex
    {
        int32 PositionIndex = -1;
        int32 NormalIndex = -1;
        int32 UVIndex = -1;
    };

    struct FIntermediateObjFace
    {
        TArray<FIntermediateObjFaceVertex> Vertices;
        FString                             MaterialName;
    };

    struct FIntermediateObjData
    {
        TArray<FVector>               Positions;
        TArray<FVector>               Colors;
        TArray<FVector>               Normals;
        TArray<FVector2>              UVs;
        TArray<FString>               MaterialLibraries;
        TArray<FIntermediateObjFace> Faces;
    };

} // namespace Asset
