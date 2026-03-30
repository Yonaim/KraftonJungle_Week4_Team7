#pragma once

#include "Core/CoreMinimal.h"

#include <filesystem>

namespace Engine::SceneIO
{
    FString NormalizeSceneAssetPath(const FString& InPath);
    std::filesystem::path ResolveSceneAssetPathToAbsolute(const FString& InPath);
} // namespace Engine::SceneIO
