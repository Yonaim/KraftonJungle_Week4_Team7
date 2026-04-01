#include "Viewport/EditorViewportClient.h"

#include "ApplicationCore/Input/InputRouter.h"
#include "Editor/EditorContext.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/World.h"
#include "Engine/Game/Actor.h"

#include "imgui.h"
#include "Renderer/RendererModule.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/SceneRenderData.h"

void FEditorViewportClient::Create()
{
    InputRouter = new Engine::ApplicationCore::FInputRouter();
    InputRouter->AddContext(&ViewportInputContext);
    InputRouter->AddContext(&SelectionInputContext);
    InputRouter->AddContext(&GizmoInputContext);
    GizmoController.SetViewportClient(this);
    GizmoController.SetViewportSelectionController(&SelectionController);
    SelectionController.SetViewportClient(this);

    NavigationController.SetCamera(&ViewportCamera);
    NavigationController.SetSelectionController(&SelectionController);
    SelectionController.SetCamera(&ViewportCamera);
    GizmoController.SetCamera(&ViewportCamera);
    GizmoInputContext.SetNavigationController(&NavigationController);

    ViewportCamera.SetProjectionType(EViewportProjectionType::Perspective);
    ViewportCamera.SetFOV(3.141592f * 0.5f);
    ViewportCamera.SetNearPlane(0.1f);
    ViewportCamera.SetFarPlane(2000.0f);
    ViewportCamera.SetLocation(FVector(-20.0f, 1.0f, 10.0f));
    ViewportCamera.SetRotation(FRotator(0.0f, 0.0f, 0.0f));
}

void FEditorViewportClient::Release()
{
    if (InputRouter)
    {
        delete InputRouter;
        InputRouter = nullptr;
    }
}

void FEditorViewportClient::Initialize(FWorld* World, uint32 ViewportWidth, uint32 ViewportHeight)
{
    SetWorld(World);

    ViewportCamera.OnResize(ViewportWidth, ViewportHeight);

    FScene* ActiveScene = (World != nullptr) ? World->GetActiveScene() : nullptr;
    SelectionController.SetActors(ActiveScene != nullptr ? ActiveScene->GetActors() : nullptr);
    SelectionController.SetCamera(&ViewportCamera);
    SelectionController.SetViewportSize(ViewportWidth, ViewportHeight);
}

void FEditorViewportClient::Tick(float DeltaTime, const Engine::ApplicationCore::FInputState& State)
{
    ViewportInputContext.SetDeltaTime(DeltaTime);

    if (InputRouter)
    {
        InputRouter->Tick(State);
    }

    if (CurWorld != nullptr)
    {
        FScene* ActiveScene = CurWorld->GetActiveScene();
        SelectionController.SetActors(ActiveScene != nullptr ? ActiveScene->GetActors() : nullptr);
    }
    else
    {
        SelectionController.SetActors(nullptr);
    }

    SelectionController.SyncSelectionFromContext();
    NavigationController.Tick(DeltaTime);
}

void FEditorViewportClient::Draw() {}

void FEditorViewportClient::HandleInputEvent(const Engine::ApplicationCore::FInputEvent& Event,
                                             const Engine::ApplicationCore::FInputState& State)
{
    if (InputRouter)
    {
        InputRouter->RouteEvent(Event, State);
    }
}

void FEditorViewportClient::HandleGizmoRenderCommand(FEditorRenderData& OutEditorRenderData, FSceneRenderData& OutSceneRenderData)
{
    if (!SelectionController.GetSelectedActors().empty())
    {
        OutEditorRenderData.Gizmo.GizmoType = GizmoController.GetGizmoType();
        OutEditorRenderData.Gizmo.Highlight = GizmoController.GetGizmoHighlight();
        GizmoController.SetSelectedActor(SelectionController.GetSelectedActors().back());
        if (GizmoController.bIsWorldMode && GizmoController.GetGizmoType() != EGizmoType::Scaling)
        {
            FVector RelativeLocation{
                GizmoController.GetSelectedActor()->GetRootComponent()->GetRelativeLocation()};
            OutEditorRenderData.Gizmo.Frame = FMatrix::MakeTranslation(RelativeLocation);
        }
        else
        {
            OutEditorRenderData.Gizmo.Frame =
                GizmoController.GetSelectedActor()->GetRootComponent()->GetRelativeMatrixNoScale();
        }
        GizmoController.GizmoScale =
            (ViewportCamera.GetLocation() -
             GizmoController.GetSelectedActor()->GetRootComponent()->GetRelativeLocation())
            .Size() /
            10.f;
        //  Size 여기서 조정
        OutEditorRenderData.Gizmo.Scale = GizmoController.GizmoScale;
        GizmoController.bIsDrawed = OutEditorRenderData.bShowGizmo;

        // Gizmo Render Commands
        if (OutEditorRenderData.bShowGizmo && OutEditorRenderData.Gizmo.GizmoType != EGizmoType::None)
        {
            if (SelectionController.GetEditorContext() && SelectionController.GetEditorContext()->Renderer)
            {
                FGeneralRenderer* GeneralRenderer = SelectionController.GetEditorContext()->Renderer->GetGeneralRenderer();
                if (GeneralRenderer)
                {
                    const auto& GizmoRes = GeneralRenderer->GetGizmoResources();
                    const auto& GizmoDraw = OutEditorRenderData.Gizmo;
                    FMatrix     World = FMatrix::MakeScale(GizmoDraw.Scale) * GizmoDraw.Frame;

                    const TArray<FGizmoMeshPart>* Parts = nullptr;
                    switch (GizmoDraw.GizmoType)
                    {
                    case EGizmoType::Translation: Parts = &GizmoRes.TranslationParts; break;
                    case EGizmoType::Rotation:    Parts = &GizmoRes.RotationParts;    break;
                    case EGizmoType::Scaling:     Parts = &GizmoRes.ScaleParts;       break;
                    }

                    if (Parts)
                    {
                        for (const auto& Part : *Parts)
                        {
                            FRenderCommand Cmd;
                            Cmd.MeshData = Part.MeshData.get();
                            Cmd.WorldMatrix = World;
                            Cmd.Material = FGeneralRenderer::GetDefaultMaterial();
                            Cmd.RenderLayer = ERenderLayer::Overlay;
                            Cmd.ObjectId = Part.PickId;

                            // Highlight Check
                            EGizmoType DecodedType;
                            EAxis      DecodedAxis;
                            bool       bIsHovered = false;
                            if (PickId::DecodeGizmoPart(Part.PickId, DecodedType, DecodedAxis))
                            {
                                if (DecodedAxis == EAxis::Center)
                                    bIsHovered = (GizmoDraw.Highlight == EGizmoHighlight::Center);
                                else if (DecodedAxis == EAxis::X)
                                    bIsHovered = (GizmoDraw.Highlight == EGizmoHighlight::X);
                                else if (DecodedAxis == EAxis::Y)
                                    bIsHovered = (GizmoDraw.Highlight == EGizmoHighlight::Y);
                                else if (DecodedAxis == EAxis::Z)
                                    bIsHovered = (GizmoDraw.Highlight == EGizmoHighlight::Z);
                            }
                            
                            if (bIsHovered)
                            {
                                // Solid Yellow
                                Cmd.MultiplyColor = FVector4(0.0f, 0.0f, 0.0f, 1.0f);
                                Cmd.AdditiveColor = FVector4(1.0f, 1.0f, 0.0f, 1.0f);
                            }
                            else
                            {
                                Cmd.MultiplyColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
                                Cmd.AdditiveColor = FVector4(0.0f, 0.0f, 0.0f, 1.0f);
                            }
                            
                            Cmd.DepthStencilOption.DepthEnable = true;
                            Cmd.DepthStencilOption.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
                            Cmd.DepthStencilOption.DepthFunc = D3D11_COMPARISON_ALWAYS;
                            Cmd.RasterizerOption.CullMode = D3D11_CULL_NONE;

                            OutSceneRenderData.RenderCommands.push_back(Cmd);
                        }
                    }
                }
            }
        }
    }
    else
    {
        OutEditorRenderData.Gizmo.GizmoType = EGizmoType::None;
        OutEditorRenderData.Gizmo.Highlight = EGizmoHighlight::None;
        OutEditorRenderData.Gizmo.Frame = FMatrix::Identity;
        OutEditorRenderData.Gizmo.Scale = 1.0f;
        GizmoController.SetSelectedActor(nullptr);
        GizmoController.bIsDrawed = false;
    }
}

void FEditorViewportClient::BuildRenderData(FEditorRenderData& OutEditorRenderData,
                                            FSceneRenderData&  OutSceneRenderData,
                                            EEditorShowFlags   InShowFlags)
{
    // SceneView 업데이트
    SceneView.SetViewMatrix(ViewportCamera.GetViewMatrix());
    SceneView.SetProjectionMatrix(ViewportCamera.GetProjectionMatrix());
    SceneView.SetViewLocation(ViewportCamera.GetLocation());
    SceneView.SetClipPlanes(ViewportCamera.GetNearPlane(), ViewportCamera.GetFarPlane());
    SceneView.OnResize({0, 0, (int32)ViewportCamera.GetWidth(), (int32)ViewportCamera.GetHeight()});
    OutSceneRenderData.SceneView = &SceneView;
    
    switch(RenderSetting.GetViewMode())
    {
    case EViewModeIndex::VMI_Lit:
    case EViewModeIndex::VMI_Unlit:
        break;      // Do nothing
    case EViewModeIndex::VMI_Wireframe:
        for (auto& RC : OutSceneRenderData.RenderCommands)
        {
            if (RC.bIgnoreWireFrame) 
                continue;
            RC.RasterizerOption.FillMode = D3D11_FILL_WIREFRAME;
        }
        break;
    }

    // EditorView 업데이트
    OutEditorRenderData.bShowGrid = 
                                IsFlagSet(InShowFlags, EEditorShowFlags::SF_Grid);
    OutEditorRenderData.bShowWorldAxes = 
                                IsFlagSet(InShowFlags, EEditorShowFlags::SF_WorldAxes);
    OutEditorRenderData.bShowSelectionOutline =
                                IsFlagSet(InShowFlags, EEditorShowFlags::SF_SelectionOutline);
    OutEditorRenderData.bShowGizmo = 
                                IsFlagSet(InShowFlags, EEditorShowFlags::SF_Gizmo);

    // 기즈모 렌더 커맨드 생성
    // 260401 NOTE: 기즈모 '정보'는 OutEditorRenderData, '렌더 커맨드'는 OutSceneRenderData에 나눠져있음... 
    // \(^o^)/
    HandleGizmoRenderCommand(OutEditorRenderData, OutSceneRenderData);
}

void FEditorViewportClient::OnResize(uint32 Width, uint32 Height)
{
    ViewportCamera.OnResize(Width, Height);
    SelectionController.SetViewportSize(Width, Height);
}

void FEditorViewportClient::SetEditorContext(FEditorContext* InContext)
{
    SelectionController.SetEditorContext(InContext);
}

void FEditorViewportClient::SetWorld(FWorld* InWorld)
{
    CurWorld = InWorld;
    FScene* ActiveScene = (CurWorld != nullptr) ? CurWorld->GetActiveScene() : nullptr;
    SelectionController.SetActors(ActiveScene != nullptr ? ActiveScene->GetActors() : nullptr);
}

void FEditorViewportClient::SyncSelectionFromContext()
{
    SelectionController.SyncSelectionFromContext();
}

void FEditorViewportClient::DrawViewportOverlay()
{
    if (!SelectionController.IsDraggingSelection())
    {
        return;
    }

    int32 StartX, StartY, EndX, EndY;
    SelectionController.GetSelectionRect(StartX, StartY, EndX, EndY);

    const float MinX = (float)std::min(StartX, EndX);
    const float MinY = (float)std::min(StartY, EndY);
    const float MaxX = (float)std::max(StartX, EndX);
    const float MaxY = (float)std::max(StartY, EndY);

    ImDrawList* DrawList = ImGui::GetForegroundDrawList();
    DrawList->AddRectFilled(ImVec2(MinX, MinY), ImVec2(MaxX, MaxY), IM_COL32(80, 140, 255, 40));
    DrawList->AddRect(ImVec2(MinX, MinY), ImVec2(MaxX, MaxY), IM_COL32(80, 140, 255, 255), 0.0f, 0,
                      1.5f);
}
