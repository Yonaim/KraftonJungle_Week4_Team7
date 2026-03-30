#include "PrimitivePlane.h"
#include "Core/Misc/Paths.h"

const FString CPrimitivePlane::Key = "Plane";

// FString CPrimitivePlane::GetFilePath() { return FPaths::MeshDir() + "Plane.mesh"; }

CPrimitivePlane::CPrimitivePlane()
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

void CPrimitivePlane::Generate()
{
	auto Data = std::make_shared<FMeshData>();

	FColor White = { 1.0f, 1.0f, 1.0f, 1.0f };
	FVector Normal = { 0.0f, 1.0f, 0.0f };

	Data->Vertices.push_back({ { -5.0f,  5.0f, 0.0f }, Normal, White });
	Data->Vertices.push_back({ {  5.0f,  5.0f, 0.0f }, Normal, White });
	Data->Vertices.push_back({ {  5.0f, -5.0f, 0.0f }, Normal, White });
	Data->Vertices.push_back({ { -5.0f, -5.0f, 0.0f }, Normal, White });

	Data->Indices.push_back(0);
	Data->Indices.push_back(2);
	Data->Indices.push_back(1);
	Data->Indices.push_back(0);
	Data->Indices.push_back(3);
	Data->Indices.push_back(2);

	Data->Topology = EMeshTopology::EMT_TriangleList;

	MeshData = Data;
	RegisterMeshData(Key, Data);
}
