#include "PrimitiveBase.h"
#include <fstream>
#include <cstdio>

#include "Renderer/D3D11/D3D11RHI.h"

// ─── FMeshData ───

void FMeshData::Bind(FD3D11RHI* Context)
{
    UINT Stride = sizeof(FPrimitiveVertex);
    UINT Offset = 0;
    Context->SetVertexBuffer(0, VertexBuffer.get(), Stride, Offset);
    Context->SetIndexBuffer(IndexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);
}

void FMeshData::UpdateLocalBound()
{
    if (Vertices.empty())
    {
        MinCoord = FVector(FLT_MAX, FLT_MAX, FLT_MAX);
        MaxCoord = FVector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        LocalBoundRadius = 0.f;
    }
    else
    {
        // TODO: Ritter's Algorithm으로 개선
        for (const FPrimitiveVertex& Vertex : Vertices)
        {
            if (Vertex.Position.X < MinCoord.X)
                MinCoord.X = Vertex.Position.X;
            if (Vertex.Position.X > MaxCoord.X)
                MaxCoord.X = Vertex.Position.X;
            if (Vertex.Position.Y < MinCoord.Y)
                MinCoord.Y = Vertex.Position.Y;
            if (Vertex.Position.Y > MaxCoord.Y)
                MaxCoord.Y = Vertex.Position.Y;
            if (Vertex.Position.Z < MinCoord.Z)
                MinCoord.Z = Vertex.Position.Z;
            if (Vertex.Position.Z > MaxCoord.Z)
                MaxCoord.Z = Vertex.Position.Z;

            LocalBoundRadius = ((MaxCoord - MinCoord) * 0.5).Size();
        }
    }
}

// ─── CPrimitiveBase ───

std::unordered_map<FString, std::shared_ptr<FMeshData>> CPrimitiveBase::MeshCache;

std::shared_ptr<FMeshData> CPrimitiveBase::LoadFromFile(const FString& Key, const FString& FilePath)
{
    auto It = MeshCache.find(Key);
    if (It != MeshCache.end())
    {
        return It->second;
    }

    auto Data = LoadFromFile(FilePath);
    if (Data)
    {
        MeshCache[Key] = Data;
        RegisterMeshData(Key, Data);
    }
    return Data;
}

std::shared_ptr<FMeshData> CPrimitiveBase::GetCached(const FString& Key)
{
    auto It = MeshCache.find(Key);
    if (It != MeshCache.end())
    {
        return It->second;
    }
    return nullptr;
}

void CPrimitiveBase::RegisterMeshData(const FString& Key, std::shared_ptr<FMeshData> Data)
{
    Data->UpdateLocalBound();
    MeshCache[Key] = Data;
}

void CPrimitiveBase::ClearCache()
{
    MeshCache.clear();
}

// 바이너리 파일 포맷:
// [uint32] VertexCount
// [FPrimitiveVertex * VertexCount] Vertices
// [uint32] IndexCount
// [uint32 * IndexCount] Indices
std::shared_ptr<FMeshData> CPrimitiveBase::LoadFromFile(const FString& FilePath)
{
    std::ifstream File(FilePath, std::ios::binary);
    if (!File.is_open())
    {
        printf("[PrimitiveBase] Failed to open mesh file: %s\n", FilePath.c_str());
        return nullptr;
    }

    auto Data = std::make_shared<FMeshData>();

    uint32_t VertexCount = 0;
    File.read(reinterpret_cast<char*>(&VertexCount), sizeof(uint32_t));
    Data->Vertices.resize(VertexCount);
    File.read(reinterpret_cast<char*>(Data->Vertices.data()),
              VertexCount * sizeof(FPrimitiveVertex));

    uint32_t IndexCount = 0;
    File.read(reinterpret_cast<char*>(&IndexCount), sizeof(uint32_t));
    Data->Indices.resize(IndexCount);
    File.read(reinterpret_cast<char*>(Data->Indices.data()), IndexCount * sizeof(uint32_t));

    // 메시 읽어올 때 기본 옵션 Triangle list
    Data->Topology = EMeshTopology::EMT_TriangleList;

    if (File.fail())
    {
        printf("[PrimitiveBase] Failed to read mesh file: %s\n", FilePath.c_str());
        return nullptr;
    }

    printf("[PrimitiveBase] Loaded mesh: %s (Vertices: %u, Indices: %u)\n",
           FilePath.c_str(), VertexCount, IndexCount);

    return Data;
}