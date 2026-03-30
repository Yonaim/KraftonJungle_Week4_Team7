#pragma once

#include "Core/CoreMinimal.h"

#include <filesystem>

namespace Engine::Scene::Serialization
{
    ENGINE_API FString NormalizeSceneAssetPath(const FString& InPath);
    ENGINE_API std::filesystem::path ResolveSceneAssetPathToAbsolute(const FString& InPath);
} // namespace Engine::Scene::Serialization
