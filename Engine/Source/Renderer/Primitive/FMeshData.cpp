#include "FMeshData.h"

#include "Renderer/D3D11/D3D11RHI.h"

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
