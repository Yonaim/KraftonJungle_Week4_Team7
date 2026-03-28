#pragma once

#include "Core/CoreMinimal.h"

#include "Viewport/EditorViewportClient.h"
#include "Viewport/Layout/EditorViewportLayout.h"
#include "Renderer/SceneView.h"

class SEditorViewportTab
{
public:
    SEditorViewportTab();
    ~SEditorViewportTab();

public:
    void Construct();
    void OnResize(FViewportRect WindowRect);

    TArray<FSceneView*> const& GetViewports() const { return SceneViews; }
    FSceneView* const&         GetViewport(int32 index) const { return SceneViews[index]; }

    void CreateExtraViewportClients();
    void RemoveExtraViewportClients();

private:
    TArray<FSceneView*>            SceneViews;
    TArray<FEditorViewportClient*> ViewportClients;
    FEditorViewportLayout*         ViewportLayout = nullptr;
};