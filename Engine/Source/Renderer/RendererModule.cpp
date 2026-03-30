#include "Renderer/RendererModule.h"

#include "SceneView.h"
#include "Engine/Component/Mesh/LineBatchComponent.h"
#include "Renderer/Types/PickId.h"
#include "Renderer/Types/PickResult.h"

FRendererModule::FRendererModule()
{
}

FRendererModule::~FRendererModule()
{
}

bool FRendererModule::StartupModule(HWND hWnd)
{
    if (hWnd == nullptr)
    {
        return false;
    }
    
    RECT ClientRect = {};
    if (!GetClientRect(hWnd, &ClientRect))
    {
        return false;
    }
    int ViewportWidth = static_cast<int32>(ClientRect.right - ClientRect.left);
    int ViewportHeight = static_cast<int32>(ClientRect.bottom - ClientRect.top);
    
    GeneralRenderer = new FGeneralRenderer(hWnd, ViewportWidth, ViewportHeight);
    
    if (GeneralRenderer->GetDevice() != nullptr)
    {
#if defined(_DEBUG)
        GeneralRenderer->GetDevice()->QueryInterface(__uuidof(ID3D11Debug),
                                        reinterpret_cast<void**>(DebugDevice.GetAddressOf()));
#endif
    }

    return true;
}

void FRendererModule::ShutdownModule()
{
    DebugDevice.Reset();

#if defined(_DEBUG)
    if (DebugDevice != nullptr)
    {
        DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        DebugDevice.Reset();
    }
#endif

    if (GeneralRenderer)
    {
        delete GeneralRenderer;
        GeneralRenderer = nullptr;
    }
}

void FRendererModule::BeginFrame()
{
    GeneralRenderer->BeginFrame();
}

void FRendererModule::EndFrame()
{
    GeneralRenderer->EndFrame();
}

void FRendererModule::SetViewport(const D3D11_VIEWPORT& InViewport) 
{ 
    GeneralRenderer->GetRHI().SetViewport(InViewport); 
}

void FRendererModule::OnWindowResized(int32 InWidth, int32 InHeight)
{
    if (GeneralRenderer)
    {
        GeneralRenderer->OnResize(InWidth, InHeight);
    }
}

void FRendererModule::Render(const FEditorRenderData& InEditorRenderData,
                             const FSceneRenderData&  InSceneRenderData)
{
    RenderWorldPass(InEditorRenderData, InSceneRenderData);
    RenderOverlayPass(InEditorRenderData, InSceneRenderData);
}

void FRendererModule::RenderWorldPass(const FEditorRenderData& InEditorRenderData,
                                      const FSceneRenderData&  InSceneRenderData)
{
    if (!InSceneRenderData.RenderCommands.empty())
    {
        FRenderCommandQueue CommandQueue;
        for (auto el : InSceneRenderData.RenderCommands)
        {
            CommandQueue.AddCommand(el);
        }
        
        if (InEditorRenderData.SceneView)
        {
            CommandQueue.ViewMatrix = InEditorRenderData.SceneView->GetViewMatrix();
            CommandQueue.ProjectionMatrix = InEditorRenderData.SceneView->GetProjectionMatrix();
        }
        
        GeneralRenderer->SubmitCommands(CommandQueue);
        GeneralRenderer->ExecuteCommands();
    }
}

void FRendererModule::RenderOverlayPass(const FEditorRenderData& InEditorRenderData,
                                        const FSceneRenderData&  InSceneRenderData)
{
    // Gizmo etc. would be rendered here using GeneralRenderer commands if needed.
}

bool FRendererModule::Pick(const FEditorRenderData& InEditorRenderData,
                           const FSceneRenderData&  InSceneRenderData,
                           int32 MouseX, int32 MouseY,
                           FPickResult& OutResult)
{
    if (!GeneralRenderer) return false;
    
    uint32 PickId = 0;
    if (GeneralRenderer->Pick(MouseX, MouseY, PickId))
    {
        OutResult.ObjectId = PickId;
        return PickId != 0;
    }
    return false;
}

void FRendererModule::SetVSyncEnabled(bool bEnabled) 
{ 
    if (GeneralRenderer) GeneralRenderer->SetVSync(bEnabled); 
}

bool FRendererModule::IsVSyncEnabled() const 
{ 
    return GeneralRenderer ? GeneralRenderer->IsVSyncEnabled() : false; 
}
