#include "Renderer/D3D11/D3D11TextBatchRenderer.h"

bool FD3D11TextBatchRenderer::Initialize(FD3D11RHI* InRHI)
{
    (void)InRHI;
    PendingTextItems.clear();
    CurrentSceneView = nullptr;
    return true;
}

void FD3D11TextBatchRenderer::Shutdown()
{
    PendingTextItems.clear();
    CurrentSceneView = nullptr;
}

void FD3D11TextBatchRenderer::BeginFrame()
{
    PendingTextItems.clear();
    CurrentSceneView = nullptr;
}

void FD3D11TextBatchRenderer::BeginFrame(const FSceneView* InSceneView)
{
    PendingTextItems.clear();
    CurrentSceneView = InSceneView;
}

void FD3D11TextBatchRenderer::AddText(const FTextRenderItem& InItem)
{
    PendingTextItems.push_back(InItem);
}

void FD3D11TextBatchRenderer::AddTexts(const TArray<FTextRenderItem>& InItems)
{
    PendingTextItems.insert(PendingTextItems.end(), InItems.begin(), InItems.end());
}

void FD3D11TextBatchRenderer::EndFrame(const FSceneView* InSceneView)
{
    Flush(InSceneView);
}

void FD3D11TextBatchRenderer::Flush(const FSceneView* InSceneView)
{
    (void)InSceneView;
    PendingTextItems.clear();
}
