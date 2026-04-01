#include "Core/Misc/Paths.h"

#include <cassert>
#include <system_error>

namespace fs = std::filesystem;

FPathConfig FPaths::Config{};
bool        FPaths::bInitialized = false;

bool FPaths::Initialize(const FPathConfig& InConfig)
{
    if (!ValidateConfig(InConfig))
    {
        return false;
    }

    FPathConfig NewConfig{};
    NewConfig.EngineRoot = Normalize(InConfig.EngineRoot);
    NewConfig.AppRoot = Normalize(InConfig.AppRoot);

    NewConfig.EngineContentDir = InConfig.EngineContentDir.empty()
                                     ? Combine(NewConfig.EngineRoot, L"Resources")
                                     : Normalize(InConfig.EngineContentDir);

    NewConfig.AppContentDir = InConfig.AppContentDir.empty()
                                  ? Combine(NewConfig.AppRoot, L"Content")
                                  : Normalize(InConfig.AppContentDir);

    NewConfig.SavedDir = InConfig.SavedDir.empty() ? Combine(NewConfig.AppRoot, L"Saved")
                                                   : Normalize(InConfig.SavedDir);

    NewConfig.ShaderDir = InConfig.ShaderDir.empty() ? Combine(NewConfig.AppContentDir, L"Shader")
                                                     : Normalize(InConfig.ShaderDir);

    NewConfig.ShaderCacheDir = InConfig.ShaderCacheDir.empty()
                                   ? Combine(NewConfig.SavedDir, L"ShaderCache")
                                   : Normalize(InConfig.ShaderCacheDir);

    Config = std::move(NewConfig);
    bInitialized = true;
    return true;
}

bool FPaths::IsInitialized() { return bInitialized; }

const fs::path& FPaths::EngineRoot() { return GetConfig().EngineRoot; }

const fs::path& FPaths::AppRoot() { return GetConfig().AppRoot; }

const fs::path& FPaths::EngineContentDir() { return GetConfig().EngineContentDir; }

const fs::path& FPaths::AppContentDir() { return GetConfig().AppContentDir; }

const fs::path& FPaths::SavedDir() { return GetConfig().SavedDir; }

const fs::path& FPaths::ShaderDir() { return GetConfig().ShaderDir; }

const fs::path& FPaths::ShaderCacheDir() { return GetConfig().ShaderCacheDir; }

void FPaths::EnsureRuntimeDirectories()
{
    const FPathConfig& CurrentConfig = GetConfig();

    std::error_code ErrorCode;
    fs::create_directories(CurrentConfig.SavedDir, ErrorCode);
    ErrorCode.clear();
    fs::create_directories(CurrentConfig.ShaderCacheDir, ErrorCode);
}

fs::path FPaths::Normalize(const fs::path& InPath)
{
    if (InPath.empty())
    {
        return {};
    }

    return InPath.lexically_normal();
}

fs::path FPaths::Combine(const fs::path& Base, const fs::path& Relative)
{
    return Normalize(Base / Relative);
}

fs::path FPaths::PathFromUtf8(const FString& Utf8Path)
{
    if (Utf8Path.empty())
    {
        return {};
    }

    return fs::u8path(Utf8Path);
}

FString FPaths::Utf8FromPath(const fs::path& Path)
{
    if (Path.empty())
    {
        return {};
    }

    const std::u8string Utf8Path = Path.u8string();
    return FString(reinterpret_cast<const char*>(Utf8Path.data()), Utf8Path.size());
}

const FPathConfig& FPaths::GetConfig()
{
    assert(bInitialized && "FPaths must be initialized before use.");
    return Config;
}

bool FPaths::ValidateConfig(const FPathConfig& InConfig)
{
    return !InConfig.EngineRoot.empty() && !InConfig.AppRoot.empty();
}