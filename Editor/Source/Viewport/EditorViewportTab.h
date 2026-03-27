#pragma once

#include "Core/CoreMinimal.h"

#include "Viewport/EditorViewportClient.h"
#include "Renderer/SceneView.h"

class SEditorViewportTab
{
public:
    SEditorViewportTab();
    ~SEditorViewportTab();

public:
    void Construct();

    TArray<FSceneView*> const& GetViewports() const { return SceneViews; }
    FSceneView* const&         GetViewport(int32 index) const { return SceneViews[index]; }

    void CreateExtraViewportClients();
    void RemoveExtraViewportClients();

private:
    TArray<FSceneView*>            SceneViews;
    TArray<FEditorViewportClient*> ViewportClients;
};