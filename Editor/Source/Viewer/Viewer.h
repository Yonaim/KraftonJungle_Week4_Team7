#pragma once

#include "Renderer/RendererModule.h"
#include "ApplicationCore/Input/InputSystem.h"

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
    FSceneView*              GetViewport() const;
    const FEditorRenderData& GetEditorRenderData() const;
    const FSceneRenderData&  GetSceneRenderData() const;

    void DrawPanel();

    // OBJ 로드 등 추가 기능 필요시 public 함수로 제공

  private:
    // 내부 상태
    FSceneView*       Viewport = nullptr;
    FEditorRenderData EditorRenderData;
    FSceneRenderData  SceneRenderData;

    FD3D11RHI* RHI = nullptr;

    // 카메라, 입력, Mesh 등 Viewer에 필요한 멤버 추가
    // 예: MeshData, Camera, UI 상태 등
};