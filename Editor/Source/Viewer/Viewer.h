#pragma once

#include "Renderer/RendererModule.h"
#include "ApplicationCore/Input/InputSystem.h"
#include "Camera/ViewportCamera.h"

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
    FSceneView*              GetSceneView() const;
    const FSceneRenderData&  GetSceneRenderData() const;

    void DrawPanel();

    // OBJ 로드 등 추가 기능 필요시 public 함수로 제공

  private:
    // 내부 상태
    FSceneView*       SceneView = nullptr;
    FSceneRenderData  SceneRenderData;
    FViewportCamera   ViewportCamera;

    FD3D11RHI* RHI = nullptr;

};