#include "StaticMesh.h"
#include "Core/Path.h"

FString UStaticMesh::GetMeshName() const
{
    if (StaticMeshAsset == nullptr || StaticMeshAsset->PathFileName.empty())
    {
        return "None";
    }

    const std::filesystem::path FilePath(StaticMeshAsset->PathFileName);
    return FilePath.stem().string();
}

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

uint32 UStaticMesh::GetVerticesCount() const
{
    return StaticMeshAsset ? static_cast<uint32>(StaticMeshAsset->Vertices.size()) : 0;
}

uint32 UStaticMesh::GetIndicesCount() const
{
    return StaticMeshAsset ? static_cast<uint32>(StaticMeshAsset->Indices.size()) : 0;
}

void UStaticMesh::Build()
{
    // 에셋 로드 완료 후 호출
    CalculateAABB();
}

bool UStaticMesh::IsValidLowLevel() const
{
    if (StaticMeshAsset == nullptr)
    {
        return false;
    }

    if (StaticMeshAsset->Vertices.empty() || StaticMeshAsset->Indices.empty())
    {
        return false;
    }

    return true;
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
