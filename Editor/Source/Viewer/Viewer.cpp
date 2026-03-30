#include "Viewer.h"
#include "Renderer/SceneView.h"

#include "imgui.h"
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

FViewer::FViewer() {}

FViewer::~FViewer() { Release(); }

void FViewer::Create()
{
    SceneView = new FSceneView();

    FPrimitiveRenderItem item;

    item.MeshType = EBasicMeshType::Cube;
    item.World = FMatrix::Identity;


    ViewportCamera.SetFOV(3.141592f * 0.5f);
    ViewportCamera.SetNearPlane(0.1f);
    ViewportCamera.SetFarPlane(2000.0f);
    ViewportCamera.SetLocation(FVector(-5.0f, 0.0f, 1.0f));
    ViewportCamera.SetRotation(FRotator::ZeroRotator);

    SceneView->SetViewLocation(ViewportCamera.GetLocation());
    SceneView->SetViewMatrix(ViewportCamera.GetViewMatrix());
    SceneView->SetProjectionMatrix(ViewportCamera.GetProjectionMatrix());

    SceneRenderData.SceneView = SceneView;
    SceneRenderData.Primitives.clear();
    SceneRenderData.Primitives.push_back(item);
}

void FViewer::Release()
{
    // 리소스 해제
     delete SceneView; 
     SceneView = nullptr;
}

void FViewer::SetRuntimeServices(FD3D11RHI* InRHI)
{
    RHI = InRHI;
    // 필요시 리소스 로더 등 초기화
}

void FViewer::Tick(float DeltaTime, Engine::ApplicationCore::FInputSystem* InputSystem)
{
    // 입력 처리
    // 예: 카메라 입력 → Viewport 갱신
    // Mesh/OBJ 데이터 갱신 필요시 처리
}

void FViewer::OnWindowResized(float Width, float Height)
{
    // Viewport 크기 갱신
    if (SceneView)
    {
        SceneView->OnResize({0, 0, static_cast<int32>(Width), static_cast<int32>(Height)});
        //ViewportCamera.OnResize(Width, Height);
    }
}

FSceneView* FViewer::GetSceneView() const { return SceneView; }

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