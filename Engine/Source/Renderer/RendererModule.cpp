#include "Renderer/RendererModule.h"

#include "SceneView.h"
#include "Engine/Component/Mesh/LineBatchComponent.h"
#include "Renderer/Types/PickId.h"
#include "Renderer/Types/PickResult.h"

namespace
{
    bool HasScenePrimitives(const FSceneRenderData& InSceneRenderData)
    {
        return InSceneRenderData.SceneView != nullptr && !InSceneRenderData.Primitives.empty();
    }

    bool ShouldRenderSelectionOutline(const FEditorRenderData& InEditorRenderData,
                                      const FSceneRenderData&  InSceneRenderData)
    {
        return HasScenePrimitives(InSceneRenderData) && InEditorRenderData.bShowSelectionOutline &&
               InSceneRenderData.ViewMode != EViewModeIndex::VMI_Wireframe;
    }

    bool ShouldTintSelectedWireframe(const FEditorRenderData& InEditorRenderData,
                                     const FSceneRenderData&  InSceneRenderData)
    {
        return HasScenePrimitives(InSceneRenderData) && InEditorRenderData.bShowSelectionOutline &&
               InSceneRenderData.ViewMode == EViewModeIndex::VMI_Wireframe;
    }

    TArray<FPrimitiveRenderItem>
    BuildWireframePrimitiveSubmission(const FSceneRenderData& InSceneRenderData)
    {
        TArray<FPrimitiveRenderItem> SubmissionItems;
        SubmissionItems.reserve(InSceneRenderData.Primitives.size());

        for (const FPrimitiveRenderItem& Item : InSceneRenderData.Primitives)
        {
            SubmissionItems.push_back(Item);
        }

        const FColor SelectionColor = FD3D11OutlineRenderer::GetVisibleOutlineColor();

        for (FPrimitiveRenderItem& Item : SubmissionItems)
        {
            if (Item.State.IsSelected())
            {
                Item.Color = SelectionColor;
            }
        }

        return SubmissionItems;
    }
} // namespace

FRendererModule::FRendererModule()
{
}

FRendererModule::~FRendererModule()
{
}

bool FRendererModule::StartupModule(HWND hWnd)
{
    if (hWnd == nullptr)
    {
        return false;
    }
    
    if (!RHI.Initialize(hWnd))
    {
        ShutdownModule();
        return false;
    }
    
    // === GeneralRenderer 초기화
    RECT ClientRect = {};
    if (!GetClientRect(hWnd, &ClientRect))
    {
        return false;
    }
    int ViewportWidth = static_cast<int32>(ClientRect.right - ClientRect.left);
    int ViewportHeight = static_cast<int32>(ClientRect.bottom - ClientRect.top);
    GeneralRenderer = new FGeneralRenderer(hWnd, ViewportWidth, ViewportHeight);
    GeneralRenderer->DEBUG_ForceInitialize(
        RHI.GetDevice(),
        RHI.GetDeviceContext(),
        RHI.GetBackBufferRTV(),
        RHI.GetDepthStencilView(), 
        RHI.GetViewport(),
        RHI.GetSwapChain()
    );
    // === GeneralRenderer 초기화

    if (!MeshBatchRenderer.Initialize(&RHI))
    {
        ShutdownModule();
        return false;
    }

    if (!OutlineRenderer.Initialize(&RHI))
    {
        ShutdownModule();
        return false;
    }

    if (!ObjectIdRenderer.Initialize(&RHI))
    {
        ShutdownModule();
        return false;
    }

#if defined(_DEBUG)
    if (RHI.GetDevice() != nullptr)
    {
        RHI.GetDevice()->QueryInterface(__uuidof(ID3D11Debug),
                                        reinterpret_cast<void**>(DebugDevice.GetAddressOf()));
    }
#endif

    return true;
}

void FRendererModule::ShutdownModule()
{
    DebugDevice.Reset();

    ObjectIdRenderer.Shutdown();
    MeshBatchRenderer.Shutdown();

#if defined(_DEBUG)
    if (DebugDevice != nullptr)
    {
        DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        DebugDevice.Reset();
    }
#endif

    RHI.Shutdown();
}

void FRendererModule::BeginFrame()
{
    RHI.BeginFrame();

    static const FLOAT ClearColor[4] = {0.17f, 0.17f, 0.17f, 1.0f};

    RHI.SetDefaultRenderTargets();
    RHI.Clear(ClearColor, 1.0f, 0);
    
    // === GeneralRenderer 사용한 렌더 루트
    GeneralRenderer->SetSceneRenderTarget(RHI.GetBackBufferRTV(), nullptr, RHI.GetViewport());
    GeneralRenderer->BeginFrame();
}

// 테스트용 임시 플래그
bool hasRenderCommand = false;

void FRendererModule::EndFrame()
{
    RHI.EndFrame();
    // if (hasRenderCommand)
    //     GeneralRenderer->EndFrame();
}

void FRendererModule::SetViewport(const D3D11_VIEWPORT& InViewport) { RHI.SetViewport(InViewport); }

void FRendererModule::OnWindowResized(int32 InWidth, int32 InHeight)
{
    if (InWidth <= 0 || InHeight <= 0)
    {
        return;
    }

    // 1. RHI의 리소스를 먼저 갱신 (자원 관리 주체)
    RHI.Resize(InWidth, InHeight);

    // 2. 갱신된 리소스를 GeneralRenderer에 전달 (참조만 수행)
    GeneralRenderer->OnResize(InWidth, InHeight);
    GeneralRenderer->DEBUG_UpdateViewResources(RHI.GetBackBufferRTV(), RHI.GetDepthStencilView(), RHI.GetViewport());

    ObjectIdRenderer.Resize(InWidth, InHeight);
}

/**
 * @brief Render Order: World Pass -> Overlay Pass
 */
void FRendererModule::Render(const FEditorRenderData& InEditorRenderData,
                             const FSceneRenderData&  InSceneRenderData)
{
    RenderWorldPass(InEditorRenderData, InSceneRenderData);
    RenderOverlayPass(InEditorRenderData, InSceneRenderData);
}

void FRendererModule::RenderWorldPass(const FEditorRenderData& InEditorRenderData,
                                      const FSceneRenderData&  InSceneRenderData)
{
    // === GeneralRenderer 사용한 렌더 루트
    if (!InSceneRenderData.RenderCommands.empty())
    {
        FRenderCommandQueue CommandQueue;
        for (auto el : InSceneRenderData.RenderCommands)
        {
            CommandQueue.AddCommand(el);
        }
        // ==== 테스트 코드. 추후 삭제할 것
        {
            CommandQueue.ViewMatrix = InEditorRenderData.SceneView->GetViewMatrix();
            CommandQueue.ProjectionMatrix = InEditorRenderData.SceneView->GetProjectionMatrix();
        }
        // ==== 테스트 코드. 추후 삭제할 것
        
        GeneralRenderer->SubmitCommands(CommandQueue);
        GeneralRenderer->ExecuteCommands();
    }
    // === GeneralRenderer 사용한 렌더 루트
        
    if (HasScenePrimitives(InSceneRenderData))
    {
        FMeshBatchPassParams ScenePassParams = {};
        ScenePassParams.SceneView = InSceneRenderData.SceneView;
        ScenePassParams.ViewMode = InSceneRenderData.ViewMode;
        ScenePassParams.bUseInstancing = InSceneRenderData.bUseInstancing;
        ScenePassParams.bDisableDepth = false;

        MeshBatchRenderer.BeginFrame(ScenePassParams);

        if (ShouldTintSelectedWireframe(InEditorRenderData, InSceneRenderData))
        {
            const TArray<FPrimitiveRenderItem> WireframeItems =
                BuildWireframePrimitiveSubmission(InSceneRenderData);
            MeshBatchRenderer.AddPrimitives(WireframeItems);
        }
        else
        {
            SceneMeshSubmitter.Submit(MeshBatchRenderer, InSceneRenderData);
        }

        MeshBatchRenderer.EndFrame();
    }
}

/**
 * Overlay: Gizmo -> Text
 */
void FRendererModule::RenderOverlayPass(const FEditorRenderData& InEditorRenderData,
                                        const FSceneRenderData&  InSceneRenderData)
{
    const bool bHasEditorGizmo =
        InEditorRenderData.SceneView != nullptr && InEditorRenderData.bShowGizmo &&
        InEditorRenderData.Gizmo.GizmoType != EGizmoType::None;

    if (bHasEditorGizmo)
    {
        RHI.ClearDepthStencil(RHI.GetDepthStencilView(), 1.0f, 0);

        // ================= Gizmo =================
        FMeshBatchPassParams GizmoPassParams = {};
        GizmoPassParams.SceneView = InEditorRenderData.SceneView;
        GizmoPassParams.ViewMode = EViewModeIndex::VMI_Unlit;
        GizmoPassParams.bUseInstancing = true;
        GizmoPassParams.bDisableDepth = false;

        MeshBatchRenderer.BeginFrame(GizmoPassParams);
        OverlayMeshSubmitter.Submit(MeshBatchRenderer, InEditorRenderData);
        MeshBatchRenderer.EndFrame();

        // ================= Gizmo Center =================
        FMeshBatchPassParams GizmoCenterPassParams = GizmoPassParams;
        GizmoCenterPassParams.bDisableDepth = true;

        MeshBatchRenderer.BeginFrame(GizmoCenterPassParams);
        OverlayMeshSubmitter.SubmitCenterHandle(MeshBatchRenderer, InEditorRenderData);
        MeshBatchRenderer.EndFrame();
    }
}

bool FRendererModule::PickRaw(const FEditorRenderData& InEditorRenderData, int32 MouseX,
                              int32 MouseY, uint32& OutPickId)
{
    OutPickId = PickId::None;

    const FSceneView* SceneView = InEditorRenderData.SceneView;
    if (SceneView == nullptr)
    {
        return false;
    }

    ObjectIdRenderer.BeginFrame(SceneView, MouseX, MouseY);

    if (InEditorRenderData.bShowGizmo && InEditorRenderData.Gizmo.GizmoType != EGizmoType::None)
    {
        OverlayMeshSubmitter.Submit(ObjectIdRenderer, InEditorRenderData);
    }

    return ObjectIdRenderer.RenderAndReadBack(OutPickId);
}

bool FRendererModule::Pick(const FEditorRenderData& InEditorRenderData, int32 MouseX, int32 MouseY,
                           FPickResult& OutResult)
{
    OutResult = {};

    uint32 PickedId = PickId::None;
    if (!PickRaw(InEditorRenderData, MouseX, MouseY, PickedId))
    {
        return false;
    }

    OutResult = PickResult::FromPickId(PickedId);
    return true;
}

void FRendererModule::SetVSyncEnabled(bool bEnabled) { RHI.SetVSyncEnabled(bEnabled); }

bool FRendererModule::IsVSyncEnabled() const { return RHI.IsVSyncEnabled(); }
