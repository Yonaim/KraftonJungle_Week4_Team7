#include "ViewerEngineLoop.h"
#include "Viewer/Viewer.h"
#include "Renderer/SceneView.h"

#include <windows.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

bool FViewerEngineLoop::PreInit(HINSTANCE HInstance, uint32 NCmdShow)
{
    (void)HInstance;
    (void)NCmdShow;

    InputSystem = new Engine::ApplicationCore::FInputSystem();

#if defined(_WIN32)
    Application = Engine::ApplicationCore::FWindowsApplication::Create();
    if (Application == nullptr)
    {
        return false;
    }

    Engine::ApplicationCore::FWindowsApplication* WindowsApplication = GetWindowsApplication();
    if (WindowsApplication == nullptr)
    {
        return false;
    }

    if (!WindowsApplication->CreateEditorWindow(L"OBJ Viewer", 1280, 720))
    {
        return false;
    }

    Application->SetInputSystem(InputSystem);
#else
    return false;
#endif

    Viewer = new FViewer();
    Viewer->Create();

    Renderer = new FRendererModule();
    if (Renderer == nullptr)
    {
        return false;
    }

    HWND WindowHandle = static_cast<HWND>(Application->GetNativeWindowHandle());
    if (WindowHandle == nullptr)
    {
        return false;
    }

    if (!Renderer->StartupModule(WindowHandle))
    {
        return false;
    }

    Viewer->SetRuntimeServices(&Renderer->GetRHI());

    ImGui::CreateContext();
    ImGuiIO& IO = ImGui::GetIO();
#ifdef IMGUI_HAS_DOCK
    IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif
    ImGui_ImplWin32_Init((void*)WindowHandle);
    ImGui_ImplDX11_Init(Renderer->GetRHI().GetDevice(), Renderer->GetRHI().GetDeviceContext());

    if (Engine::ApplicationCore::FWindowsApplication* WindowsApplication = GetWindowsApplication())
    {
        WindowsApplication->SetMessageHandler(nullptr, nullptr);
    }

    CachedWindowWidth = Application->GetWindowWidth();
    CachedWindowHeight = Application->GetWindowHeight();
    Viewer->OnWindowResized(static_cast<float>(CachedWindowWidth),
                            static_cast<float>(CachedWindowHeight));

    InitializeForTime();

    return true;
}

int32 FViewerEngineLoop::Run()
{
    while (!bIsExit)
    {
        if (bIsExit)
        {
            break;
        }

        Tick();
    }

    return 0;
}

void FViewerEngineLoop::ShutDown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (Viewer != nullptr)
    {
        Viewer->SetRuntimeServices(nullptr);
    }

    if (Renderer != nullptr)
    {
        Renderer->ShutdownModule();
        delete Renderer;
        Renderer = nullptr;
    }

    if (Viewer != nullptr)
    {
        Viewer->Release();
        delete Viewer;
        Viewer = nullptr;
    }

    if (Application != nullptr)
    {
        Application->DestroyApplicationWindow();
        delete Application;
        Application = nullptr;
    }

    delete InputSystem;
    InputSystem = nullptr;
}

void FViewerEngineLoop::Tick()
{
    Application->PumpMessages();
    if (Application->IsExitRequested())
    {
        bIsExit = true;
        return;
    }

    RunFrameOnce();
}

void FViewerEngineLoop::InitializeForTime()
{
    PrevTime = FPlatformTime::Seconds();
    DeltaTime = 0.0f;
}

bool FViewerEngineLoop::HandleWindowResize()
{
    const int32 CurrentWindowWidth = Application->GetWindowWidth();
    const int32 CurrentWindowHeight = Application->GetWindowHeight();
    if (CurrentWindowWidth == CachedWindowWidth && CurrentWindowHeight == CachedWindowHeight)
    {
        return false;
    }

    CachedWindowWidth = CurrentWindowWidth;
    CachedWindowHeight = CurrentWindowHeight;

    if (Renderer != nullptr)
    {
        Renderer->OnWindowResized(CurrentWindowWidth, CurrentWindowHeight);
    }

    if (Viewer != nullptr)
    {
        Viewer->OnWindowResized(static_cast<float>(CurrentWindowWidth),
                                static_cast<float>(CurrentWindowHeight));
    }

    return true;
}

bool FViewerEngineLoop::RunFrameOnce()
{
    HandleWindowResize();
    return RunFrameOnceWithoutResize();
}

bool FViewerEngineLoop::RunFrameOnceWithoutResize()
{
    if (bIsRenderingDuringSizeMove)
    {
        return false;
    }

    if (Application == nullptr || Viewer == nullptr || Renderer == nullptr ||
        InputSystem == nullptr)
    {
        return false;
    }

    if (Application->IsExitRequested())
    {
        bIsExit = true;
        return false;
    }

    bIsRenderingDuringSizeMove = true;

    InputSystem->BeginFrame();
    UpdateFrameTiming();

    Viewer->Tick(DeltaTime, InputSystem);

    Renderer->BeginFrame();

    // Viewer에서 Viewport, RenderData 등은 Viewer 내부에서 관리한다고 가정
    Renderer->SetViewport(Viewer->GetViewport()->GetViewport());
    Renderer->Render(Viewer->GetEditorRenderData(), Viewer->GetSceneRenderData());

    Viewer->DrawPanel();
    Renderer->EndFrame();

    bIsRenderingDuringSizeMove = false;
    return true;
}

void FViewerEngineLoop::UpdateFrameTiming()
{
    const double CurrentTime = FPlatformTime::Seconds();
    double       RawDeltaTime = CurrentTime - PrevTime;
    PrevTime = CurrentTime;

    if (RawDeltaTime < (1.0 / 1000.0))
    {
        RawDeltaTime = 1.0 / 1000.0;
    }
    else if (RawDeltaTime > (1.0 / 15.0))
    {
        RawDeltaTime = 1.0 / 15.0;
    }

    DeltaTime = static_cast<float>(RawDeltaTime);
    MainLoopFPS = static_cast<float>(1.0 / RawDeltaTime);
}

Engine::ApplicationCore::FWindowsApplication* FViewerEngineLoop::GetWindowsApplication() const
{
#if defined(_WIN32)
    return static_cast<Engine::ApplicationCore::FWindowsApplication*>(Application);
#else
    return nullptr;
#endif
}