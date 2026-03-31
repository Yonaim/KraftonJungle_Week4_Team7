#pragma once

#include "Renderer/RendererModule.h"
#include "ApplicationCore/Input/InputSystem.h"
#include "Camera/ViewportCamera.h"
#include "ViewerNavigationController.h"
#include "Engine/Game/StaticMeshActor.h"

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

    void BuildRenderCommand();
    void DrawPanel(HWND hWnd);

  public:
    std::function<void()> OnRequestExit;

  private:
    FSceneView*      SceneView = nullptr;
    FSceneRenderData SceneRenderData;
    FViewportCamera  ViewportCamera;

    FViewerNavigationController NavigationController;

    AStaticMeshActor* TestMeshActor = nullptr;

    FD3D11RHI* RHI = nullptr;
};