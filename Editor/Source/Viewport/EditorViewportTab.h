#pragma once

#include "Core/CoreMinimal.h"

#include "Viewport/EditorViewportClient.h"
#include "Viewport/Layout/EditorViewportLayout.h"
#include "Viewport/Layout/EditorViewportLayoutFactory.h"

#include "Renderer/SceneView.h"
#include "Panel/ControlPanel.h"

class SEditorViewportTab
{
public:
    SEditorViewportTab();
    ~SEditorViewportTab();

public:
    void Construct();
    void Initialize();
    void OnResize(FViewportRect WindowRect, bool Force = 0);

    TArray<FSceneView*> const& GetViewports() const { return SceneViews; }
    FSceneView* const&         GetViewport(int32 index) const { return SceneViews[index]; }

    EViewportLayoutType GetCurrentLayoutType() { return CurrentLayoutType; }
    void SetLayout(EViewportLayoutType NewType);

    void AdjustViewportCount(EViewportLayoutType NewType);

    // UI
    void InitializeControlPanels(FEditorContext* Context);
    void DrawControlPanels();

private:
    FViewportRect                  CurrentRect = {0, 0, 0, 0};
    TArray<FSceneView*>            SceneViews;
    TArray<FEditorViewportClient*> ViewportClients;

    FEditorViewportLayout*         ViewportLayout = nullptr;
    EViewportLayoutType            CurrentLayoutType;

    TArray<FControlPanel*> ControlPanels;
};