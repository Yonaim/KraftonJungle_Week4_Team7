#pragma once

#include "Core/CoreMinimal.h"

#include "Viewport/Viewport.h"
#include "Viewport/Layout/EditorViewportLayout.h"
#include "Viewport/Layout/EditorViewportLayoutFactory.h"

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

    TArray<FViewport*> const& GetViewports() const { return Viewports; }
    FViewport* const&         GetViewport(int32 index) const { return Viewports[index]; }

    EViewportLayoutType GetCurrentLayoutType() { return CurrentLayoutType; }
    void SetLayout(EViewportLayoutType NewType);

    void AdjustViewportCount(EViewportLayoutType NewType);

    // UI
    void InitializeControlPanels(FEditorContext* Context);
    void DrawControlPanels();
    void DrawSplitters(SSplitter* Splitter, SSplitter* Parent);

private:
    FViewportRect                  CurrentRect = {0, 0, 0, 0};
    TArray<FViewport*>             Viewports;
    TArray<FEditorViewportClient*> ViewportClients;

    FEditorViewportLayout*         ViewportLayout = nullptr;
    EViewportLayoutType            CurrentLayoutType;

    TArray<FControlPanel*> ControlPanels;
    SSplitter*             DraggingSplitter = nullptr;
};