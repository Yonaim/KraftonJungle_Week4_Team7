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

    ViewportCamera.SetFOV(3.141592f * 0.5f);
    ViewportCamera.SetNearPlane(0.1f);
    ViewportCamera.SetFarPlane(2000.0f);
    ViewportCamera.SetLocation(FVector(-3.0f, 0.0f, 0.0f));
    ViewportCamera.SetRotation(FRotator::ZeroRotator);

    NavigationController.SetCamera(&ViewportCamera);
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

    const Engine::ApplicationCore::FInputState& InputState = InputSystem->GetInputState();
    const FVector2 MousePosition = {static_cast<float>(InputState.MouseX),
                                    static_cast<float>(InputState.MouseY)};

    Engine::ApplicationCore::FInputEvent InputEvent;
    while (InputSystem->PollEvent(InputEvent))
    {
        // 더블 클릭 이벤트 처리
        if (InputEvent.Type == Engine::ApplicationCore::EInputEventType::MouseDoubleClick &&
            InputEvent.Key == Engine::ApplicationCore::EKey::MouseLeft)
        {
            NavigationController.ResetView(FVector(-3, 0, 0), FVector::Zero());
        }
    }

    // Orbit (좌클릭)
    if (InputState.IsKeyDown(Engine::ApplicationCore::EKey::MouseLeft))
    {
        if (!NavigationController.IsOrbiting())
        {
            NavigationController.BeginOrbit(MousePosition);
        }

        NavigationController.UpdateOrbit(MousePosition);
    }
    else if (NavigationController.IsOrbiting())
    {
        NavigationController.EndOrbit();
    }

    // Pan (우클릭)
    if (InputState.IsKeyDown(Engine::ApplicationCore::EKey::MouseRight))
    {
        if (!NavigationController.IsPanning())
            NavigationController.BeginPan(MousePosition);
        else
            NavigationController.UpdatePan(MousePosition);
    }
    else if (NavigationController.IsPanning())
    {
        NavigationController.EndPan();
    }

    // Zoom (휠)
    float WheelDelta = InputState.WheelDelta;

    if (WheelDelta != 0.0f)
    {
        NavigationController.Zoom(WheelDelta);
    }

    NavigationController.Tick(DeltaTime);

    FPrimitiveRenderItem item;

    item.MeshType = EBasicMeshType::Cube;
    item.World = FMatrix::Identity;

    SceneView->SetViewLocation(ViewportCamera.GetLocation());
    SceneView->SetViewMatrix(ViewportCamera.GetViewMatrix());
    SceneView->SetProjectionMatrix(ViewportCamera.GetProjectionMatrix());

    SceneRenderData.SceneView = SceneView;
}

void FViewer::OnWindowResized(float Width, float Height)
{
    // Viewport 크기 갱신
    if (SceneView)
    {
        SceneView->OnResize({0, 0, static_cast<int32>(Width), static_cast<int32>(Height)});
        // ViewportCamera.OnResize(Width, Height);
    }
}

FSceneView* FViewer::GetSceneView() const { return SceneView; }

const FSceneRenderData& FViewer::GetSceneRenderData() const { return SceneRenderData; }

void FViewer::DrawPanel(HWND hWnd)
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("WantCaptureMouse: %s", io.WantCaptureMouse ? "true" : "false");
    ImGui::Text("MouseDown[0]: %s", io.MouseDown[0] ? "true" : "false");

    ImGui::ShowDemoWindow();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::MenuItem("Open OBJ..."))
        { /* 파일 열기 */
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 60);
        if (ImGui::MenuItem("Exit"))
        {
            OnRequestExit();
        }
        ImGui::EndMainMenuBar();
    }
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
