#include "Viewer.h"
#include "Renderer/SceneView.h"
#include "Engine/Component/Mesh/StaticMeshComponent.h"
#include "Engine/Asset/StaticMesh.h"
#include "Engine/Scene/SceneAssetBinder.h"
#include "RHI/DynamicRHI.h"

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
    ViewportCamera.SetLocation(FVector(3.0f, 0.0f, 0.0f));
    ViewportCamera.SetRotation(FRotator::ZeroRotator);

    NavigationController.SetCamera(&ViewportCamera);

    StaticMeshActor = new AStaticMeshActor();

    StaticMeshActor->SetLocation(FVector(0, 0, 0));
}

void FViewer::Release()
{
    delete StaticMeshActor;
    StaticMeshActor = nullptr;

    RHI = nullptr;
    DynamicRHI = nullptr;
    AssetCacheManager = nullptr;

    delete SceneView;
    SceneView = nullptr;
}

void FViewer::SetRuntimeServices(FD3D11RHI* InRHI, RHI::FDynamicRHI* InDynamicRHI,
                                 Asset::FAssetCacheManager* InAssetCacheManager)
{
    RHI = InRHI;
    DynamicRHI = InDynamicRHI;
    AssetCacheManager = InAssetCacheManager;
}

void FViewer::Tick(float DeltaTime, Engine::ApplicationCore::FInputSystem* InputSystem)
{
    SceneRenderData.RenderCommands.clear();

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

    BuildRenderData();
}

void FViewer::OnWindowResized(float Width, float Height)
{
    // Viewport 크기 갱신
    if (SceneView)
    {
        SceneView->OnResize({0, 0, static_cast<int32>(Width), static_cast<int32>(Height)});
    }
}

FSceneView* FViewer::GetSceneView() const { return SceneView; }

const FSceneRenderData& FViewer::GetSceneRenderData() const { return SceneRenderData; }

void FViewer::BuildRenderData()
{
    SceneView->SetViewLocation(ViewportCamera.GetLocation());
    SceneView->SetViewMatrix(ViewportCamera.GetViewMatrix());
    SceneView->SetProjectionMatrix(ViewportCamera.GetProjectionMatrix());

    SceneRenderData.SceneView = SceneView;
    StaticMeshActor->GetStaticMeshComponent()->CollectRenderData(SceneRenderData,
                                                               ESceneShowFlags::SF_Primitives);
}

void FViewer::DrawPanel(HWND hWnd)
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open OBJ..."))
            {
                std::array<wchar_t, 1024> FileBuffer{};
                OPENFILENAMEW             Dialog = {};
                Dialog.lStructSize = sizeof(Dialog);
                Dialog.hwndOwner = hWnd;
                Dialog.lpstrFilter = L"OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
                Dialog.lpstrFile = FileBuffer.data();
                Dialog.nMaxFile = static_cast<DWORD>(FileBuffer.size());
                Dialog.Flags =
                    OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

                if (GetOpenFileNameW(&Dialog))
                {
                    FWString SelectedPath = FileBuffer.data();
                    if (TryLoadObjFile(SelectedPath))
                    {
                        FSceneAssetBinder::BindActor(StaticMeshActor, AssetCacheManager, DynamicRHI);
                        NavigationController.ResetView(FVector(-3, 0, 0), FVector::Zero());
                    }
                }
            }
            ImGui::EndMenu();
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

FString WideToUtf8(const FWString& InText)
{
    if (InText.empty())
    {
        return {};
    }

    const int RequiredSize = WideCharToMultiByte(
        CP_UTF8, 0, InText.c_str(), static_cast<int>(InText.size()), nullptr, 0, nullptr, nullptr);
    if (RequiredSize <= 0)
    {
        return {};
    }

    FString OutText(static_cast<size_t>(RequiredSize), '\0');
    WideCharToMultiByte(CP_UTF8, 0, InText.c_str(), static_cast<int>(InText.size()), OutText.data(),
                        RequiredSize, nullptr, nullptr);
    return OutText;
}

bool FViewer::TryLoadObjFile(const FWString& FilePath)
{
    if (StaticMeshActor == nullptr)
    {
        return false;
    }
    Engine::Component::UStaticMeshComponent* StaticMeshComponent =
        StaticMeshActor->GetStaticMeshComponent();

    if (StaticMeshComponent == nullptr)
    {
        return false;
    }

    FString Utf8Path = WideToUtf8(FilePath);
    StaticMeshComponent->SetStaticMeshPath(Utf8Path);
    return true;
}
