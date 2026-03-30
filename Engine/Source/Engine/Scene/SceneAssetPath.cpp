#include "Engine/Scene/SceneAssetPath.h"

#include <algorithm>
#include <cstring>
#include <system_error>

#include "Core/Misc/Paths.h"

namespace Engine::Scene
{
    namespace
    {
        constexpr const char* GameVirtualRoot = "/Game";

        FString PathToUtf8String(const std::filesystem::path& Path)
        {
            const std::u8string Utf8Path = Path.u8string();
            return FString(reinterpret_cast<const char*>(Utf8Path.data()), Utf8Path.size());
        }

        FString NormalizeSlashCopy(const FString& InValue)
        {
            FString Result = InValue;
            std::replace(Result.begin(), Result.end(), '\\', '/');
            return Result;
        }

        std::filesystem::path NormalizeAbsolutePath(const std::filesystem::path& InPath)
        {
            if (InPath.empty())
            {
                return {};
            }

            std::error_code       ErrorCode;
            std::filesystem::path CanonicalPath =
                std::filesystem::weakly_canonical(InPath, ErrorCode);
            if (ErrorCode)
            {
                ErrorCode.clear();
                CanonicalPath = InPath.lexically_normal();
            }

            return CanonicalPath;
        }

        std::filesystem::path GetNormalizedContentRoot()
        {
            return NormalizeAbsolutePath(FPaths::AppContentDir());
        }
    } // namespace

    FString BuildSceneAssetVirtualPath(const std::filesystem::path& InPath)
    {
        if (InPath.empty())
        {
            return {};
        }

        std::filesystem::path CandidatePath = InPath;
        if (CandidatePath.is_relative())
        {
            CandidatePath = FPaths::AppContentDir() / CandidatePath;
        }

        const std::filesystem::path NormalizedContentRoot = GetNormalizedContentRoot();
        const std::filesystem::path NormalizedCandidate = NormalizeAbsolutePath(CandidatePath);

        std::error_code       ErrorCode;
        std::filesystem::path RelativePath =
            std::filesystem::relative(NormalizedCandidate, NormalizedContentRoot, ErrorCode);
        if (ErrorCode)
        {
            ErrorCode.clear();
            RelativePath = NormalizedCandidate.lexically_relative(NormalizedContentRoot);
        }

        if (RelativePath.empty())
        {
            return {};
        }

        if (RelativePath == ".")
        {
            return GameVirtualRoot;
        }

        FString VirtualPath = GameVirtualRoot;
        for (const std::filesystem::path& Part : RelativePath)
        {
            if (Part.empty() || Part == ".")
            {
                continue;
            }

            VirtualPath += "/";
            VirtualPath += PathToUtf8String(Part);
        }

        return VirtualPath;
    }

    FString NormalizeSceneAssetPath(const FString& InPath)
    {
        if (InPath.empty())
        {
            return {};
        }

        const FString SlashNormalized = NormalizeSlashCopy(InPath);

        if (SlashNormalized.rfind(GameVirtualRoot, 0) == 0)
        {
            if (SlashNormalized.size() == std::strlen(GameVirtualRoot))
            {
                return GameVirtualRoot;
            }

            FString Trimmed = SlashNormalized;
            while (Trimmed.size() > std::strlen(GameVirtualRoot) && Trimmed.back() == '/')
            {
                Trimmed.pop_back();
            }

            return Trimmed;
        }

        return BuildSceneAssetVirtualPath(std::filesystem::path(SlashNormalized));
    }

    std::filesystem::path ResolveSceneAssetPathToAbsolute(const FString& InPath)
    {
        if (InPath.empty())
        {
            return {};
        }

        const FString SlashNormalized = NormalizeSlashCopy(InPath);

        if (SlashNormalized.rfind(GameVirtualRoot, 0) == 0)
        {
            FString RelativePath = SlashNormalized.substr(std::strlen(GameVirtualRoot));

            while (!RelativePath.empty() && RelativePath.front() == '/')
            {
                RelativePath.erase(RelativePath.begin());
            }

            std::filesystem::path AbsolutePath = FPaths::AppContentDir();
            if (!RelativePath.empty())
            {
                AbsolutePath /= std::filesystem::path(RelativePath);
            }

            return NormalizeAbsolutePath(AbsolutePath);
        }

        std::filesystem::path AbsolutePath = std::filesystem::path(SlashNormalized);
        if (AbsolutePath.is_relative())
        {
            AbsolutePath = FPaths::AppContentDir() / AbsolutePath;
        }

        return NormalizeAbsolutePath(AbsolutePath);
    }

} // namespace Engine::Scene