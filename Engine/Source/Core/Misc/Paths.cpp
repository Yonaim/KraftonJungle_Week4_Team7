#include "Core/Misc/Paths.h"

#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#endif

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

const FPathConfig& FPaths::GetConfig()
{
    assert(bInitialized && "FPaths must be initialized before use.");
    return Config;
}

bool FPaths::ValidateConfig(const FPathConfig& InConfig)
{
    return !InConfig.EngineRoot.empty() && !InConfig.AppRoot.empty();
}

FWString FPaths::PathFromUtf8(const FString& Utf8Path)
{
    if (Utf8Path.empty())
    {
        return {};
    }

#ifdef _WIN32
    // Windows std::filesystem::path는 wide path를 쓰는 쪽이 안전함.
    const int WideLength = MultiByteToWideChar(CP_UTF8, 0, Utf8Path.c_str(), -1, nullptr, 0);

    if (WideLength <= 0)
    {
        throw std::runtime_error("Failed to convert UTF-8 path to wide string.");
    }

    std::wstring WidePath;
    WidePath.resize(static_cast<size_t>(WideLength - 1));

    const int ConvertedLength =
        MultiByteToWideChar(CP_UTF8, 0, Utf8Path.c_str(), -1, WidePath.data(), WideLength);

    if (ConvertedLength <= 0)
    {
        throw std::runtime_error("Failed to convert UTF-8 path to wide string.");
    }

    return std::filesystem::path(WidePath);
#else
    // Linux/macOS에서는 UTF-8 narrow path로도 보통 잘 동작
    return std::filesystem::path(Utf8Path);
#endif
}

FWString FPaths::PathFromUtf8(const char* Utf8Path)
{
    return PathFromUtf8(Utf8Path ? FString(Utf8Path) : FString());
}