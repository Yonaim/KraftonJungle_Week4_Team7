#pragma once
#include "CoreUObject/Object.h"

// 임시로 쓰겠습니다1. 나중에 바꿔주세요
struct FNormalVertex
{
    FVector  pos;
    FVector  normal;
    FVector4 color;
    FVector2 tex;
};
// 임시로 쓰겠습니다2
struct FStaticMesh
{
    std::string PathFileName;

    TArray<FNormalVertex> Vertices;
    TArray<uint32>        Indices;
};

class UStaticMesh : public UObject
{
  public:
    const FString& GetAssetPathFileName() { return StaticMeshAsset->PathFileName; }
    void           SetStaticMeshAsset(FStaticMesh* InStaticMesh) { StaticMeshAsset = InStaticMesh; }

    const TArray<FNormalVertex>& GetVerticesData() const;
    const TArray<uint32>&        GetIndicesData() const;

    const Geometry::FAABB GetLocalAABB() const { return CachedAABB; }

    void Build();

  private:
    void CalculateAABB();

  private:
    FStaticMesh*    StaticMeshAsset = nullptr;
    Geometry::FAABB CachedAABB;
};