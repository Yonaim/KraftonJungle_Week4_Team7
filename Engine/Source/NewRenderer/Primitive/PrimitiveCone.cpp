#include "PrimitiveCone.h"
#include "Resources/Mesh/Cone.h"

const FString CPrimitiveCone::Key = "Cone";

CPrimitiveCone::CPrimitiveCone()
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

void CPrimitiveCone::Generate()
{
	auto Data = std::make_shared<FMeshData>();

	FColor Color = { 1.0f, 1.0f, 1.0f, 1.0f };
	FVector Normal = { 0.0f, 1.0f, 0.0f }; // 임시 노멀

	for (uint32_t i = 0; i < cone_vertex_count; ++i)
	{
		Data->Vertices.push_back({ { cone_vertices[i].x, cone_vertices[i].y, cone_vertices[i].z }, Color, Normal });
	}

	for (uint32_t i = 0; i < cone_index_count; ++i)
	{
		Data->Indices.push_back(cone_indices[i]);
	}

	Data->Topology = EMeshTopology::EMT_TriangleList;

	MeshData = Data;
	RegisterMeshData(Key, Data);
}
