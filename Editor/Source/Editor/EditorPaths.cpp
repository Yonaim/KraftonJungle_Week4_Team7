#include "Editor/EditorPaths.h"
#include "Core/Misc/Paths.h"

std::filesystem::path FEditorPaths::AboutLogo()
{
    return FPaths::Combine(FPaths::AppRoot(), L"Content", L"Texture", L"Logo", L"copass.png");
}

std::filesystem::path FEditorPaths::DefaultSceneDirectory()
{
    return FPaths::Combine(FPaths::AppContentDir(), L"Scenes");
}