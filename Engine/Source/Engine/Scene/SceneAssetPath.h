#pragma once

#include "Core/CoreMinimal.h"
#include "Core/Misc/Paths.h"
#include "Core/EngineAPI.h"

namespace Engine::Scene
{
    ENGINE_API FString BuildSceneAssetVirtualPath(const std::filesystem::path& InPath);
    ENGINE_API FString NormalizeSceneAssetPath(const FString& InPath);
    ENGINE_API std::filesystem::path ResolveSceneAssetPathToAbsolute(const FString& InPath);
} // namespace Engine::Scene