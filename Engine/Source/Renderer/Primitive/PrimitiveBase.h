#pragma once

#include "Renderer/Types/VertexTypes.h"
#include "RHI/RHIBuffer.h"

// Forward Declaration
class FD3D11RHI;
// Forward Declaration

enum class EMeshTopology
{
    EMT_Undefined = 0, // = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED
    EMT_Point = 1, // =  D3D11_PRIMITIVE_TOPOLOGY_POINTLIST
    EMT_LineList = 2, // = D3D11_PRIMITIVE_TOPOLOGY_LINELIST
    EMT_LineStrip = 3, // = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
    EMT_TriangleList = 4, // = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    EMT_TriangleStrip = 5, // = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
};

struct ENGINE_API FMeshData
{
    FMeshData() : SortId(NextSortId++)
    {
    }

    // NOTE: 이제 Mesh도 에셋 관리자로 중앙집중식 관리하므로 
    //      FMeshData가 직접 Release하지 않음.
    ~FMeshData() { /*Release();*/ }

    uint32 GetSortId() const { return SortId; }
    bool   bIsDirty = true; // 최초 1회 초기화 보장

    void Bind(FD3D11RHI* Context);

    // 토폴로지 옵션
    EMeshTopology Topology = EMeshTopology::EMT_Undefined;

    // Deprecated
    // 이제 메시는 FMeshData 대신 SceneAssetBinder에서 관리함
    // PrimitiveXXX 삭제되면 이것도 삭제할 것.
    TArray<FPrimitiveVertex> Vertices;
    // Deprecated
    // 이제 메시는 FMeshData 대신 SceneAssetBinder에서 관리함
    // PrimitiveXXX 삭제되면 이것도 삭제할 것.
    TArray<uint32>           Indices;
    
    uint32 VertexBufferCount;
    uint32 IndexBufferCount;
    
    // SceneAssetBinder에서 설정됨
    std::shared_ptr<RHI::FRHIVertexBuffer> VertexBuffer = nullptr;
    std::shared_ptr<RHI::FRHIIndexBuffer> IndexBuffer = nullptr;

    /** AABB Box Extent 및 Local Bound Radius 갱신 */
    void  UpdateLocalBound();
    float GetLocalBoundRadius() const { return LocalBoundRadius; }

    FVector GetMinCoord() const { return MinCoord; }
    FVector GetMaxCoord() const { return MaxCoord; }
    FVector GetCenterCoord() const { return (MaxCoord - MinCoord) * 0.5 + MinCoord; }

private:
    uint32               SortId = 0;
    static inline uint32 NextSortId = 0;

    FVector MinCoord = FVector(FLT_MAX, FLT_MAX, FLT_MAX);
    FVector MaxCoord = FVector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    float   LocalBoundRadius = 0.f;
};

class ENGINE_API CPrimitiveBase
{
public:
    CPrimitiveBase() = default;
    virtual ~CPrimitiveBase() = default;

    FMeshData* GetMeshData() const
    {
        if (MeshData)
        {
            return MeshData.get();
        }

        return nullptr;
    }

    // 캐시에서 가져오거나 파일에서 로드
    static std::shared_ptr<FMeshData> LoadFromFile(const FString& Key, const FString& FilePath);
    // 캐시에서만 조회
    static std::shared_ptr<FMeshData> GetCached(const FString& Key);
    // 코드로 생성한 데이터를 캐시에 등록
    static void RegisterMeshData(const FString& Key, std::shared_ptr<FMeshData> Data);
    static void ClearCache();

    void    SetPrimitiveFileName(const FString& InFileName) { PrimitiveFileName = InFileName; }
    FString GetPrimitiveFileName() const { return PrimitiveFileName; }

protected:
    std::shared_ptr<FMeshData> MeshData;

private:
    static TMap<FString, std::shared_ptr<FMeshData>> MeshCache;

    static std::shared_ptr<FMeshData> LoadFromFile(const FString& FilePath);

    FString PrimitiveFileName = "";
};