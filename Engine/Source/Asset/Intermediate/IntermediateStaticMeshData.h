#pragma once

#include "Core/CoreMinimal.h"

namespace Asset
{

    struct FIntermediateMeshFaceVertex
    {
        int32 PositionIndex = -1;
        int32 NormalIndex = -1;
        int32 UVIndex = -1;
    };

    struct FIntermediateMeshFace
    {
        TArray<FIntermediateMeshFaceVertex> Vertices;
        FString                             MaterialName;
    };

    struct FIntermediateStaticMeshData
    {
        TArray<FVector>               Positions;
        TArray<FVector>               Colors;
        TArray<FVector>               Normals;
        TArray<FVector2>              UVs;
        TArray<FString>               MaterialLibraries;
        TArray<FIntermediateMeshFace> Faces;
    };

} // namespace Asset
