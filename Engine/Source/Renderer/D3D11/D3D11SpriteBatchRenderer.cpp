#include "Renderer/D3D11/D3D11SpriteBatchRenderer.h"

bool FD3D11SpriteBatchRenderer::Initialize(FD3D11RHI* InRHI)
{
    (void)InRHI;
    PendingSpriteItems.clear();
    CurrentSceneView = nullptr;
    return true;
}

void FD3D11SpriteBatchRenderer::Shutdown()
{
    PendingSpriteItems.clear();
    CurrentSceneView = nullptr;
}

void FD3D11SpriteBatchRenderer::BeginFrame()
{
    PendingSpriteItems.clear();
    CurrentSceneView = nullptr;
}

void FD3D11SpriteBatchRenderer::BeginFrame(const FSceneView* InSceneView)
{
    PendingSpriteItems.clear();
    CurrentSceneView = InSceneView;
}

void FD3D11SpriteBatchRenderer::AddSprite(const FSpriteRenderItem& InItem)
{
    PendingSpriteItems.push_back(InItem);
}

void FD3D11SpriteBatchRenderer::AddSprites(const TArray<FSpriteRenderItem>& InItems)
{
    PendingSpriteItems.insert(PendingSpriteItems.end(), InItems.begin(), InItems.end());
}

void FD3D11SpriteBatchRenderer::EndFrame(const FSceneView* InSceneView)
{
    Flush(InSceneView);
}

void FD3D11SpriteBatchRenderer::Flush(const FSceneView* InSceneView)
{
    (void)InSceneView;
    PendingSpriteItems.clear();
}
