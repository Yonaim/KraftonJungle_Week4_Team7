#pragma once

#include <cfloat>
#include <cstring>
#include <filesystem>
#include <memory>
#include <utility>

#include "Engine/Asset/Asset.h"
#include "Asset/Cooked/StaticMeshCookedData.h"
#include "Asset/Runtime/StaticMeshRenderResource.h"
#include "Core/Geometry/Primitives/AABB.h"

using namespace Asset;

class UStaticMesh : public UAsset
{
    DECLARE_RTTI(UStaticMesh, UAsset)

  public:
    // 기존 전자 인터페이스
    const std::shared_ptr<FStaticMeshCookedData>& GetCookedData() const { return CookedData; }

    void SetCookedData(std::shared_ptr<FStaticMeshCookedData> InCookedData)
    {
        CookedData = std::move(InCookedData);
    }

    const std::shared_ptr<FStaticMeshRenderResource>& GetRenderResource() const
    {
        return RenderResource;
    }

    void SetRenderResource(std::shared_ptr<FStaticMeshRenderResource> InRenderResource)
    {
        RenderResource = std::move(InRenderResource);
    }

    void ResetRenderResource()
    {
        if (RenderResource)
        {
            RenderResource->Reset();
        }
        RenderResource.reset();
    }

    // 후자 쪽 편의 인터페이스 추가
    FString GetMeshName() const;

    const TArray<uint8>&  GetVerticesData() const;
    const TArray<uint32>& GetIndicesData() const;

    uint32 GetVertexStride() const;
    uint32 GetVerticesCount() const;
    uint32 GetIndicesCount() const;

    EStaticMeshVertexFormat GetVertexFormat() const;

    void Build();
    bool IsValidLowLevel() const;

    const Geometry::FAABB& GetAABB() const { return CachedAABB; }
    void                   CalculateAABB();

  private:
    std::shared_ptr<FStaticMeshCookedData>     CookedData;
    std::shared_ptr<FStaticMeshRenderResource> RenderResource;
    Geometry::FAABB                            CachedAABB;
};
