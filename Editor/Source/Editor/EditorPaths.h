#pragma once

#include "Core/EngineAPI.h"

#include <filesystem>

class ENGINE_API FEditorPaths
{
  public:
    static std::filesystem::path AboutLogo();
    static std::filesystem::path DefaultSceneDirectory();
};