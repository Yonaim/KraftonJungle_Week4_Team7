#include "Viewer.h"
#include "Renderer/SceneView.h"

#include "imgui.h"
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

FViewer::FViewer() {}

FViewer::~FViewer() { Release(); }

void FViewer::Create()
{
    // Viewport, 카메라, Mesh 등 초기화
    Viewport = new FSceneView();
    // 필요한 경우 OBJ 파일 자동 로드 등
}

void FViewer::Release()
{
    // 리소스 해제
    // delete Viewport; Viewport = nullptr;
}

void FViewer::SetRuntimeServices(FD3D11RHI* InRHI)
{
    RHI = InRHI;
    // 필요시 리소스 로더 등 초기화
}

void FViewer::Tick(float DeltaTime, Engine::ApplicationCore::FInputSystem* InputSystem)
{
    // 입력 처리, 카메라 이동, 애니메이션 등
    // 예: 카메라 입력 → Viewport 갱신
    // Mesh/OBJ 데이터 갱신 필요시 처리
}

void FViewer::OnWindowResized(float Width, float Height)
{
    // Viewport 크기 갱신
    if (Viewport)
    {
        //Viewport->SetSize(static_cast<int>(Width), static_cast<int>(Height));
    }
}

FSceneView* FViewer::GetViewport() const { return Viewport; }

const FEditorRenderData& FViewer::GetEditorRenderData() const { return EditorRenderData; }

const FSceneRenderData& FViewer::GetSceneRenderData() const { return SceneRenderData; }

void FViewer::DrawPanel()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // 최소한의 UI: 파일 열기, 카메라 정보, 모델 정보 등
    ImGui::Begin("OBJ Viewer");
    ImGui::Text("OBJ Viewer is running.");
    // TODO: 파일 열기 버튼, 모델 정보 표시 등
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}