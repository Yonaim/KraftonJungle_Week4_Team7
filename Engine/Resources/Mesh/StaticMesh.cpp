#include "StaticMesh.h"

const TArray<FNormalVertex>& UStaticMesh::GetVerticesData() const
{
    assert(StaticMeshAsset != nullptr);
    return StaticMeshAsset->Vertices;
}

const TArray<uint32>& UStaticMesh::GetIndicesData() const
{
    assert(StaticMeshAsset != nullptr);
    return StaticMeshAsset->Indices;
}

void UStaticMesh::Build()
{
    // 에셋 로드 완료 후 호출
    CalculateAABB();
}

void UStaticMesh::CalculateAABB() 
{
    if (!StaticMeshAsset || StaticMeshAsset->Vertices.empty())
    {
        CachedAABB = Geometry::FAABB();
        return;
    }

    FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
    FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const auto& Vertex : StaticMeshAsset->Vertices)
    {
        const FVector& VertexPosition = Vertex.pos;

        Min.X = std::min(Min.X, VertexPosition.X);
        Min.Y = std::min(Min.Y, VertexPosition.Y);
        Min.Z = std::min(Min.Z, VertexPosition.Z);
                                
        Max.X = std::max(Max.X, VertexPosition.X);
        Max.Y = std::max(Max.Y, VertexPosition.Y);
        Max.Z = std::max(Max.Z, VertexPosition.Z);
    }

    CachedAABB.Min = Min;
    CachedAABB.Max = Max;
}
