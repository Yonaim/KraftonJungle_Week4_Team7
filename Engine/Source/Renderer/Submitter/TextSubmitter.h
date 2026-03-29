#pragma once

#include "Renderer/SceneRenderData.h"

namespace Engine::Component { class ULineBatchComponent; }
class FD3D11TextBatchRenderer;

class FTextSubmitter
{
  public:
    void Submit(FD3D11TextBatchRenderer& InTextRenderer,
                const FSceneRenderData&   InSceneRenderData) const;

    void Submit(Engine::Component::ULineBatchComponent& InLineRenderer,
                const FSceneRenderData&   InSceneRenderData) const;
};
