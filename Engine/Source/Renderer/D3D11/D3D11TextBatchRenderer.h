#pragma once

#include "Renderer/Types/RenderItem.h"

class FD3D11RHI;
class FSceneView;

class FD3D11TextBatchRenderer
{
  public:
    bool Initialize(FD3D11RHI* InRHI);
    void Shutdown();

    void BeginFrame();
    void BeginFrame(const FSceneView* InSceneView);

    void AddText(const FTextRenderItem& InItem);
    void AddTexts(const TArray<FTextRenderItem>& InItems);
    void EndFrame(const FSceneView* InSceneView);
    void Flush(const FSceneView* InSceneView);

  private:
    const FSceneView* CurrentSceneView = nullptr;
    TArray<FTextRenderItem> PendingTextItems;
};
