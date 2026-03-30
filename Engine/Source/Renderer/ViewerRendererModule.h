#pragma once

#include <Windows.h>
#include "Renderer/D3D11/D3D11Common.h"
#include "Renderer/D3D11/D3D11MeshBatchRenderer.h"
#include "Renderer/D3D11/D3D11LineBatchRenderer.h"
#include "Renderer/D3D11/D3D11RHI.h"
#include "Renderer/SceneRenderData.h"

// RendererModule이 editor에서만 필요한 기능을 포함하고 있어, 
// ViewerRendererModule로 분리하여 Viewer에서 사용할 수 있도록 함
// 이후 ViewerRendererModule 과 EditorRendererModule로 RendererModule을 분리하여 구현할 예정입니다.

class ENGINE_API FViewerRendererModule
{
  public:
    bool StartupModule(HWND hWnd);
    void ShutdownModule();

    void BeginFrame();
    void EndFrame();

    void SetViewport(const D3D11_VIEWPORT& InViewport);
    void OnWindowResized(int32 InWidth, int32 InHeight);

    void Render(const FSceneRenderData& SceneRenderData);

    FD3D11RHI& GetRHI() { return RHI; }

    void SetVSyncEnabled(bool bEnabled);
    bool IsVSyncEnabled() const;

  private:
    FD3D11RHI               RHI;
    FD3D11MeshBatchRenderer MeshBatchRenderer;
};
