#pragma once

#include "Asset/Core/StaticMeshTypes.h"

namespace Asset
{

struct FStaticMeshCookedData
{
    FString                 SourcePath;
    EStaticMeshVertexFormat VertexFormat = EStaticMeshVertexFormat::P;

    TArray<uint8> VertexData;
    uint32        VertexStride = 0;
    uint32        VertexCount = 0;

    TArray<uint32>                 Indices;
    TArray<FStaticMeshSectionData> Sections;

    bool bHasNormals = false;
    bool bHasColors = false;
    bool bHasUVs = false;

    bool IsValid() const
    {
        return !VertexData.empty() && VertexStride > 0 && VertexCount > 0 && !Indices.empty();
    }

    void Reset()
    {
        SourcePath.clear();
        VertexFormat = EStaticMeshVertexFormat::P;
        VertexData.clear();
        VertexStride = 0;
        VertexCount = 0;
        Indices.clear();
        Sections.clear();
        bHasNormals = false;
        bHasColors = false;
        bHasUVs = false;
    }
};

} // namespace Asset
