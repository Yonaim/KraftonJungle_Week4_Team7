#include "Viewport/EditorViewportClient.h"

#include "ApplicationCore/Input/InputRouter.h"
#include "Editor/EditorContext.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/World.h"
#include "Engine/Game/Actor.h"
#include "Engine/Component/Mesh/LineBatchComponent.h"

#include "imgui.h"
#include "Engine/EngineStatics.h"
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

    GridLineBatcher = new Engine::Component::ULineBatchComponent();
}

void FEditorViewportClient::Release()
{
    if (InputRouter)
    {
        delete InputRouter;
        InputRouter = nullptr;
    }

    if (GridLineBatcher)
    {
        delete GridLineBatcher;
        GridLineBatcher = nullptr;
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

    // 그리드 및 월드 축 렌더 커맨드 생성
    if (OutEditorRenderData.bShowGrid || OutEditorRenderData.bShowWorldAxes)
    {
        DrawWorldGrid(OutEditorRenderData, OutSceneRenderData);
    }
}

void FEditorViewportClient::DrawWorldGrid(FEditorRenderData& OutEditorRenderData,
                                          FSceneRenderData&  OutSceneRenderData)
{
    if (!GridLineBatcher)
    {
        return;
    }

    GridLineBatcher->ClearLines();

    const float GridSize = 2000.0f;
    const float GridSpacing = UEngineStatics::GridSpacing;
    const FColor GridColor(0.3f, 0.3f, 0.3f, 1.0f);
    const float  HalfSize = GridSize * 0.5f;
    const float  PersistentLifeTime = -1.0f;

    const EViewportProjectionType ProjType = ViewportCamera.GetProjectionType();
    const EViewportOrthographicType OrthoType = ViewportCamera.GetOrthographicType();

    if (ProjType == EViewportProjectionType::Perspective || OrthoType == EViewportOrthographicType::Top ||
        OrthoType == EViewportOrthographicType::Bottom)
    {
        // XY 평면 그리드
        for (float y = -GridSpacing; y >= -HalfSize; y -= GridSpacing)
            GridLineBatcher->AddLine(FVector(-HalfSize, y, 0.0f), FVector(HalfSize, y, 0.0f), GridColor,
                                     PersistentLifeTime);
        for (float y = GridSpacing; y <= HalfSize; y += GridSpacing)
            GridLineBatcher->AddLine(FVector(-HalfSize, y, 0.0f), FVector(HalfSize, y, 0.0f), GridColor,
                                     PersistentLifeTime);

        for (float x = -GridSpacing; x >= -HalfSize; x -= GridSpacing)
            GridLineBatcher->AddLine(FVector(x, -HalfSize, 0.0f), FVector(x, HalfSize, 0.0f), GridColor,
                                     PersistentLifeTime);
        for (float x = GridSpacing; x <= HalfSize; x += GridSpacing)
            GridLineBatcher->AddLine(FVector(x, -HalfSize, 0.0f), FVector(x, HalfSize, 0.0f), GridColor,
                                     PersistentLifeTime);
    }
    else if (OrthoType == EViewportOrthographicType::Front || OrthoType == EViewportOrthographicType::Back)
    {
        // YZ 평면 그리드
        for (float z = -GridSpacing; z >= -HalfSize; z -= GridSpacing)
            GridLineBatcher->AddLine(FVector(0.0f, -HalfSize, z), FVector(0.0f, HalfSize, z), GridColor,
                                     PersistentLifeTime);
        for (float z = GridSpacing; z <= HalfSize; z += GridSpacing)
            GridLineBatcher->AddLine(FVector(0.0f, -HalfSize, z), FVector(0.0f, HalfSize, z), GridColor,
                                     PersistentLifeTime);

        for (float y = -GridSpacing; y >= -HalfSize; y -= GridSpacing)
            GridLineBatcher->AddLine(FVector(0.0f, y, -HalfSize), FVector(0.0f, y, HalfSize), GridColor,
                                     PersistentLifeTime);
        for (float y = GridSpacing; y <= HalfSize; y += GridSpacing)
            GridLineBatcher->AddLine(FVector(0.0f, y, -HalfSize), FVector(0.0f, y, HalfSize), GridColor,
                                     PersistentLifeTime);
    }
    else if (OrthoType == EViewportOrthographicType::Left || OrthoType == EViewportOrthographicType::Right)
    {
        // XZ 평면 그리드
        for (float z = -GridSpacing; z >= -HalfSize; z -= GridSpacing)
            GridLineBatcher->AddLine(FVector(-HalfSize, 0.0f, z), FVector(HalfSize, 0.0f, z), GridColor,
                                     PersistentLifeTime);
        for (float z = GridSpacing; z <= HalfSize; z += GridSpacing)
            GridLineBatcher->AddLine(FVector(-HalfSize, 0.0f, z), FVector(HalfSize, 0.0f, z), GridColor,
                                     PersistentLifeTime);

        for (float x = -GridSpacing; x >= -HalfSize; x -= GridSpacing)
            GridLineBatcher->AddLine(FVector(x, 0.0f, -HalfSize), FVector(x, 0.0f, HalfSize), GridColor,
                                     PersistentLifeTime);
        for (float x = GridSpacing; x <= HalfSize; x += GridSpacing)
            GridLineBatcher->AddLine(FVector(x, 0.0f, -HalfSize), FVector(x, 0.0f, HalfSize), GridColor,
                                     PersistentLifeTime);
    }

    // 월드 축 (SF_WorldAxes 플래그가 켜져 있을 때만)
    if (OutEditorRenderData.bShowWorldAxes)
    {
        GridLineBatcher->AddLine(FVector(-HalfSize, 0.0f, 0.0f), FVector(HalfSize, 0.0f, 0.0f), FColor::Red(),
                                 PersistentLifeTime);
        GridLineBatcher->AddLine(FVector(0.0f, -HalfSize, 0.0f), FVector(0.0f, HalfSize, 0.0f), FColor::Green(),
                                 PersistentLifeTime);
        GridLineBatcher->AddLine(FVector(0.0f, 0.0f, -HalfSize), FVector(0.0f, 0.0f, HalfSize), FColor::Blue(),
                                 PersistentLifeTime);
    }

    // 그리드 라인배처의 렌더 데이터를 수집하여 OutSceneRenderData에 추가
    GridLineBatcher->CollectRenderData(OutSceneRenderData, ESceneShowFlags::SF_Primitives);
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
