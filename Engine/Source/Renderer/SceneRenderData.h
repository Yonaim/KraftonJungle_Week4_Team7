#pragma once

#include "Core/Containers/Array.h"
#include "Renderer/Types/RenderItem.h"
#include "Renderer/Types/ViewMode.h"
#include "NewRenderer/RenderCommand.h"

class FSceneView;

namespace Engine::Component { class ULineBatchComponent; }

struct FSceneRenderData
{
    const FSceneView* SceneView = nullptr;
    EViewModeIndex    ViewMode = EViewModeIndex::VMI_Lit;
    bool              bUseInstancing = true;

    TArray<FRenderCommand> RenderCommands;
    
    // Deprecated
    TArray<FPrimitiveRenderItem> Primitives;
    // Deprecated
    TArray<FSpriteRenderItem>    Sprites;
    // Deprecated
    TArray<FTextRenderItem>      Texts;
    // Deprecated
    TArray<Engine::Component::ULineBatchComponent*> LineBatchers;
};