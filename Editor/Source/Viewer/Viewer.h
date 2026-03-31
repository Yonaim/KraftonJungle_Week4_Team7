#pragma once

#include "Renderer/RendererModule.h"
#include "ApplicationCore/Input/InputSystem.h"
#include "Camera/ViewportCamera.h"
#include "ViewerNavigationController.h"

class FViewer
{
  public:
    FViewer();
    ~FViewer();

    void Create();
    void Release();
    void SetRuntimeServices(FD3D11RHI* InRHI);

    void Tick(float DeltaTime, Engine::ApplicationCore::FInputSystem* InputSystem);
    void OnWindowResized(float Width, float Height);

    // 렌더러 연동용
    FSceneView*             GetSceneView() const;
    const FSceneRenderData& GetSceneRenderData() const;

    void DrawPanel(HWND hWnd);

  public:
    std::function<void()> OnRequestExit;

  private:
    // 내부 상태
    FSceneView*      SceneView = nullptr;
    FSceneRenderData SceneRenderData;
    FViewportCamera  ViewportCamera;

    FViewerNavigationController NavigationController;

    FD3D11RHI* RHI = nullptr;
};