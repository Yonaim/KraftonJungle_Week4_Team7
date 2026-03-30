#include "Core/CoreMinimal.h"
#include "ViewerRendererModule.h"

bool FViewerRendererModule::StartupModule(HWND hWnd)
{
    if (hWnd == nullptr)
        return false;

    if (!RHI.Initialize(hWnd))
        return false;

    if (!MeshBatchRenderer.Initialize(&RHI))
        return false;

    return true;
}

void FViewerRendererModule::ShutdownModule()
{
    MeshBatchRenderer.Shutdown();
    RHI.Shutdown();
}

void FViewerRendererModule::BeginFrame()
{
    RHI.BeginFrame();
    static const FLOAT ClearColor[4] = {0.17f, 0.17f, 0.17f, 1.0f};
    RHI.SetDefaultRenderTargets();
    RHI.Clear(ClearColor, 1.0f, 0);
}

void FViewerRendererModule::EndFrame() { RHI.EndFrame(); }

void FViewerRendererModule::SetViewport(const D3D11_VIEWPORT& InViewport)
{
    RHI.SetViewport(InViewport);
}

void FViewerRendererModule::OnWindowResized(int32 InWidth, int32 InHeight)
{
    if (InWidth <= 0 || InHeight <= 0)
        return;

    RHI.Resize(InWidth, InHeight);
}

void FViewerRendererModule::Render(const FSceneRenderData& SceneRenderData)
{
    if (SceneRenderData.SceneView && !SceneRenderData.Primitives.empty())
    {
        FMeshBatchPassParams PassParams = {};
        PassParams.SceneView = SceneRenderData.SceneView;
        PassParams.ViewMode = SceneRenderData.ViewMode;
        PassParams.bUseInstancing = SceneRenderData.bUseInstancing;
        PassParams.bDisableDepth = false;

        MeshBatchRenderer.BeginFrame(PassParams);
        MeshBatchRenderer.AddPrimitives(SceneRenderData.Primitives);
        MeshBatchRenderer.EndFrame();
    }
}

void FViewerRendererModule::SetVSyncEnabled(bool bEnabled) { RHI.SetVSyncEnabled(bEnabled); }

bool FViewerRendererModule::IsVSyncEnabled() const { return RHI.IsVSyncEnabled(); }