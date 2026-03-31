#pragma once

#include "Primitive/FMeshData.h"
#include "Renderer/RenderState/RenderState.h"
#include "Engine/Asset/Material.h"


class UMaterial;

enum class ERenderLayer
{
    Default,
    Overlay,
    UI,
};

struct ENGINE_API FRenderCommand
{
    FMeshData*        MeshData = nullptr;
    FMatrix           WorldMatrix;
    UMaterial* Material = nullptr;
    uint64            SortKey = 0;
    Geometry::FAABB  WorldAABB;
    bool             bDrawAABB = false;

    ERenderLayer RenderLayer = ERenderLayer::Default;
    uint32       ObjectId = 0;

    FRasterizerStateOption   RasterizerOption;
    FDepthStencilStateOption DepthStencilOption;
    FBlendStateOption        BlendOption;
    EMeshTopology            Topology = EMeshTopology::EMT_TriangleList;

    uint32 FirstIndex = 0;
    uint32 IndexCount = 0;

    bool bIsVisible = true;
    bool bIsPickable = true;
    bool bIsSelected = false;
    bool bIsHovered = false;

    void SetStates(const UMaterial* InMaterial, EMeshTopology InTopology);
    void SetDefaultStates();

    static uint64 MakeSortKey(const UMaterial* InMaterial, const FMeshData* InMeshData);
};

/**
 * 한 프레임 동안 수집된 모든 렌더링 명령을 담는 큐
 */
struct ENGINE_API FRenderCommandQueue
{
    /** 일반 메시 렌더링 명령 목록 (텍스트, SubUV 포함 통합) */
    TArray<FRenderCommand> Commands;

    /** 프레임의 카메라 행렬 */
    FMatrix ViewMatrix;
    FMatrix ProjectionMatrix;

    void Reserve(size_t Count)
    {
        Commands.reserve(Count);
    }

    void AddCommand(const FRenderCommand& Cmd)
    {
        Commands.push_back(Cmd);
    }

    /** 큐 초기화 */
    void Clear()
    {
        Commands.clear();
    }
};