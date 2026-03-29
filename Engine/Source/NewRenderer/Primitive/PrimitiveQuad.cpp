#include "PrimitiveQuad.h"
#include "Resources/Mesh/Quad.h"

const FString CPrimitiveQuad::Key = "Quad";

CPrimitiveQuad::CPrimitiveQuad()
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

void CPrimitiveQuad::Generate()
{
	auto Data = std::make_shared<FMeshData>();

	FVector4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
	FVector Normal = { 0.0f, 1.0f, 0.0f }; // 임시 노멀

	for (uint32_t i = 0; i < quad_vertex_count; ++i)
	{
		Data->Vertices.push_back({ { quad_vertices[i].x, quad_vertices[i].y, quad_vertices[i].z }, Color, Normal });
	}

	for (uint32_t i = 0; i < quad_index_count; ++i)
	{
		Data->Indices.push_back(quad_indices[i]);
	}

	Data->Topology = EMeshTopology::EMT_TriangleList;

	MeshData = Data;
	RegisterMeshData(Key, Data);
}
