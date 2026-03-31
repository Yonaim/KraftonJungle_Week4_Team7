#include "PrimitiveCube.h"
#include "Core/Misc/Paths.h"

const FString CPrimitiveCube::Key = "Cube";

// FString CPrimitiveCube::GetFilePath() { return FPaths::MeshDir() + "Cube.mesh"; }

CPrimitiveCube::CPrimitiveCube()
{
    // 어차피 추후 CPrimtivieCube는 삭제되고 .obj 파일 로드로 대체될 것이기 때문에
    // 컴파일 에러 해결을 위해 임시로 Cache 기능 비활성화
 //    auto Cached = GetCached(Key);
 //    if (Cached)
 //    {
	// MeshData = Cached;
 //    }
 //    else
    {
	Generate();
    }
}

void CPrimitiveCube::Generate()
{
	auto Data = std::make_shared<FMeshData>();

	FColor Red = { 1.0f, 0.3f, 0.3f, 1.0f };
	FColor Green = { 0.3f, 1.0f, 0.3f, 1.0f };
	FColor Blue = { 0.3f, 0.3f, 1.0f, 1.0f };
	FColor Yellow = { 1.0f, 1.0f, 0.3f, 1.0f };
	FColor Cyan = { 0.3f, 1.0f, 1.0f, 1.0f };
	FColor Magenta = { 1.0f, 0.3f, 1.0f, 1.0f };

	// Front face (x = +0.5) — Red
	Data->Vertices.push_back({ {  0.5f, -0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f }, Red });
	Data->Vertices.push_back({ {  0.5f,  0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f }, Red });
	Data->Vertices.push_back({ {  0.5f,  0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f }, Red });
	Data->Vertices.push_back({ {  0.5f, -0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f }, Red });

	// Back face (x = -0.5) — Green
	Data->Vertices.push_back({ { -0.5f,  0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, Green });
	Data->Vertices.push_back({ { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, Green });
	Data->Vertices.push_back({ { -0.5f, -0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, Green });
	Data->Vertices.push_back({ { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, Green });

	// Top face (z = +0.5) — Blue
	Data->Vertices.push_back({ { -0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, Blue });
	Data->Vertices.push_back({ {  0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, Blue });
	Data->Vertices.push_back({ {  0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, Blue });
	Data->Vertices.push_back({ { -0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, Blue });

	// Bottom face (z = -0.5) — Yellow
	Data->Vertices.push_back({ { -0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, Yellow });
	Data->Vertices.push_back({ {  0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, Yellow });
	Data->Vertices.push_back({ {  0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, Yellow });
	Data->Vertices.push_back({ { -0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, Yellow });

	// Right face (y = +0.5) — Cyan
	Data->Vertices.push_back({ {  0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, Cyan });
	Data->Vertices.push_back({ { -0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, Cyan });
	Data->Vertices.push_back({ { -0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, Cyan });
	Data->Vertices.push_back({ {  0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, Cyan });

	// Left face (y = -0.5) — Magenta
	Data->Vertices.push_back({ { -0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, Magenta });
	Data->Vertices.push_back({ {  0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, Magenta });
	Data->Vertices.push_back({ {  0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, Magenta });
	Data->Vertices.push_back({ { -0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, Magenta });

	// 36 indices (6 faces * 2 triangles * 3 vertices)
	for (uint32 i = 0; i < 6; ++i)
	{
		uint32 Base = i * 4;
		Data->Indices.push_back(Base + 0);
		Data->Indices.push_back(Base + 1);
		Data->Indices.push_back(Base + 2);
		Data->Indices.push_back(Base + 0);
		Data->Indices.push_back(Base + 2);
		Data->Indices.push_back(Base + 3);
	}

	Data->Topology = EMeshTopology::EMT_TriangleList;
	MeshData = Data;
	// RegisterMeshData(Key, Data);
}
