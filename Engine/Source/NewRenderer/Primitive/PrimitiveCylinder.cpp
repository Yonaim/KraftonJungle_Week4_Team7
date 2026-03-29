#include "PrimitiveCylinder.h"
#include "Resources/Mesh/Cylinder.h"

const FString CPrimitiveCylinder::Key = "Cylinder";

CPrimitiveCylinder::CPrimitiveCylinder()
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

void CPrimitiveCylinder::Generate()
{
	auto Data = std::make_shared<FMeshData>();

	FVector4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
	FVector Normal = { 0.0f, 1.0f, 0.0f }; // 임시 노멀

	for (uint32_t i = 0; i < cylinder_vertex_count; ++i)
	{
		Data->Vertices.push_back({ { cylinder_vertices[i].x, cylinder_vertices[i].y, cylinder_vertices[i].z }, Color, Normal });
	}

	for (uint32_t i = 0; i < cylinder_index_count; ++i)
	{
		Data->Indices.push_back(cylinder_indices[i]);
	}

	Data->Topology = EMeshTopology::EMT_TriangleList;

	MeshData = Data;
	RegisterMeshData(Key, Data);
}
