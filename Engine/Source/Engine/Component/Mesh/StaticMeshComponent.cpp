#include "Core/CoreMinimal.h"
#include "StaticMeshComponent.h"
#include "Engine/Component/Core/ComponentProperty.h"
#include "Resources/Mesh/StaticMesh.h"

FString Engine::Component::UStaticMeshComponent::GetStaticMeshPath() const
{
    if (StaticMesh)
    {
        return StaticMesh->GetAssetPathFileName();
    }
    return MeshPath;
}

void Engine::Component::UStaticMeshComponent::SetStaticMeshPath(const FString& InPath)
{
    MeshPath = InPath; // JSON에서 읽은 경로를 일단 저장 (티켓)
}

void Engine::Component::UStaticMeshComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
{
    Builder.AddAssetPath(
        "ObjStaticMeshAsset", L"Mesh Asset", [this]() { return GetStaticMeshPath(); },
        [this](const FString& InValue) { SetStaticMeshPath(InValue); });
}

bool Engine::Component::UStaticMeshComponent::GetLocalTriangles(
    TArray<Geometry::FTriangle>& OutTriangles) const
{
    OutTriangles.clear();

    if (StaticMesh == nullptr)
    {
        return false;
    }

    const TArray<FNormalVertex>& Vertices = StaticMesh->GetVerticesData();
    const TArray<uint32>&        Indices = StaticMesh->GetIndicesData();

    OutTriangles.reserve(Indices.size() / 3);

    for (size_t i = 0; i + 2 < Indices.size(); i += 3)
    {
        // 인덱스 범위 체크 (안정성)
        uint32 I0 = Indices[i + 0];
        uint32 I1 = Indices[i + 1];
        uint32 I2 = Indices[i + 2];

        if (I0 >= Vertices.size() || I1 >= Vertices.size() || I2 >= Vertices.size())
        {
            continue;
        }

        Geometry::FTriangle Triangle;
        Triangle.V0 = Vertices[I0].pos;
        Triangle.V1 = Vertices[I1].pos;
        Triangle.V2 = Vertices[I2].pos;

        OutTriangles.push_back(Triangle);
    }

    return !OutTriangles.empty();
}

Geometry::FAABB Engine::Component::UStaticMeshComponent::GetLocalAABB() const
{
    if (StaticMesh)
    {
        return StaticMesh->GetLocalAABB();
    }

      return Geometry::FAABB(FVector(FLT_MAX, FLT_MAX, FLT_MAX),
                           FVector(-FLT_MAX, -FLT_MAX, -FLT_MAX));
}