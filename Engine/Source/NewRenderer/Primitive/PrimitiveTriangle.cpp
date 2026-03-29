#include "PrimitiveTriangle.h"
#include "Resources/Mesh/Triangle.h"

const FString CPrimitiveTriangle::Key = "Triangle";

CPrimitiveTriangle::CPrimitiveTriangle()
{
	auto Cached = GetCached(Key);
	if (Cached)
	{
		MeshData = Cached;
	}
	else
	{
		Generate();
	}
}

void CPrimitiveTriangle::Generate()
{
	auto Data = std::make_shared<FMeshData>();

	FColor Color = { 1.0f, 1.0f, 1.0f, 1.0f };
	FVector Normal = { 0.0f, 1.0f, 0.0f }; // 임시 노멀

	for (uint32_t i = 0; i < triangle_vertex_count; ++i)
	{
		Data->Vertices.push_back({ { triangle_vertices[i].x, triangle_vertices[i].y, triangle_vertices[i].z }, Color, Normal });
	}

	for (uint32_t i = 0; i < triangle_index_count; ++i)
	{
		Data->Indices.push_back(triangle_indices[i]);
	}

	Data->Topology = EMeshTopology::EMT_TriangleList;

	MeshData = Data;
	RegisterMeshData(Key, Data);
}
