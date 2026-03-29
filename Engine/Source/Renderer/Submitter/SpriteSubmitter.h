#pragma once

#include "Renderer/SceneRenderData.h"

namespace Engine::Component { class ULineBatchComponent; }
class FD3D11SpriteBatchRenderer;

class FSpriteSubmitter
{
  public:
    void Submit(FD3D11SpriteBatchRenderer& InSpriteRenderer,
                const FSceneRenderData&     InSceneRenderData) const;

    void Submit(Engine::Component::ULineBatchComponent& InLineRenderer,
                const FSceneRenderData&   InSceneRenderData) const;
};
