#include "StaticMesh.h"

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cstring>
#include <filesystem>

REGISTER_CLASS(, UStaticMesh)

FString UStaticMesh::GetMeshName() const
{
    // UAsset 쪽 이름 시스템이 있으면 그걸 쓰는 게 가장 안전
    return GetAssetName();
}

const TArray<uint8>& UStaticMesh::GetVerticesData() const
{
    assert(CookedData != nullptr);
    return CookedData->VertexData;
}

const TArray<uint32>& UStaticMesh::GetIndicesData() const
{
    assert(CookedData != nullptr);
    return CookedData->Indices;
}

uint32 UStaticMesh::GetVertexStride() const { return CookedData ? CookedData->VertexStride : 0; }

uint32 UStaticMesh::GetVerticesCount() const { return CookedData ? CookedData->VertexCount : 0; }

uint32 UStaticMesh::GetIndicesCount() const
{
    return CookedData ? static_cast<uint32>(CookedData->Indices.size()) : 0;
}

EStaticMeshVertexFormat UStaticMesh::GetVertexFormat() const
{
    return CookedData ? CookedData->VertexFormat : EStaticMeshVertexFormat::P;
}

void UStaticMesh::Build() { CalculateAABB(); }

bool UStaticMesh::IsValidLowLevel() const
{
    if (CookedData == nullptr)
    {
        return false;
    }

    if (CookedData->VertexData.empty() || CookedData->Indices.empty())
    {
        return false;
    }

    if (CookedData->VertexStride == 0 || CookedData->VertexCount == 0)
    {
        return false;
    }

    if (CookedData->VertexData.size() <
        static_cast<size_t>(CookedData->VertexStride) * CookedData->VertexCount)
    {
        return false;
    }

    return true;
}

void UStaticMesh::CalculateAABB()
{
    if (!CookedData || CookedData->VertexData.empty() ||
        CookedData->VertexStride < sizeof(FVector) || CookedData->VertexCount == 0)
    {
        CachedAABB = Geometry::FAABB();
        return;
    }

    FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
    FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    const uint8* VertexBase = CookedData->VertexData.data();
    const uint32 VertexStride = CookedData->VertexStride;
    const uint32 VertexCount = CookedData->VertexCount;

    for (uint32 i = 0; i < VertexCount; ++i)
    {
        const uint8* VertexPtr = VertexBase + static_cast<size_t>(i) * VertexStride;

        FVector Position;
        std::memcpy(&Position, VertexPtr, sizeof(FVector));

        Min.X = std::min(Min.X, Position.X);
        Min.Y = std::min(Min.Y, Position.Y);
        Min.Z = std::min(Min.Z, Position.Z);

        Max.X = std::max(Max.X, Position.X);
        Max.Y = std::max(Max.Y, Position.Y);
        Max.Z = std::max(Max.Z, Position.Z);
    }

    CachedAABB.Min = Min;
    CachedAABB.Max = Max;
}