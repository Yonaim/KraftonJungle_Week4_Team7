#pragma once

#include "Asset/Core/Types.h"

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
    TArray<FVector>               Normals;
    TArray<FVector2>              UVs;
    TArray<FIntermediateMeshFace> Faces;
};
