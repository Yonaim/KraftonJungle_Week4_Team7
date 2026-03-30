#include "PrimitiveRing.h"
#include "Resources/Mesh/Ring.h"

const FString CPrimitiveRing::Key = "Ring";

CPrimitiveRing::CPrimitiveRing()
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

void CPrimitiveRing::Generate()
{
	auto Data = std::make_shared<FMeshData>();

	FColor Color = { 1.0f, 1.0f, 1.0f, 1.0f };
	FVector Normal = { 0.0f, 1.0f, 0.0f }; // 임시 노멀

	for (uint32_t i = 0; i < ring_vertex_count; ++i)
	{
		Data->Vertices.push_back({ { ring_vertices[i].x, ring_vertices[i].y, ring_vertices[i].z }, Normal, Color });
	}

	for (uint32_t i = 0; i < ring_index_count; ++i)
	{
		Data->Indices.push_back(ring_indices[i]);
	}

	Data->Topology = EMeshTopology::EMT_TriangleList;

	MeshData = Data;
	RegisterMeshData(Key, Data);
}
