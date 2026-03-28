#pragma once

#include "Core/CoreMinimal.h"

#include <filesystem>
#include <memory>

class FScene;

class ENGINE_API FSceneSerializer
{
  public:
    static bool Serialize(const FScene& Scene, FString& OutJson,
                          FString* OutErrorMessage = nullptr);
    static bool SaveToFile(const FScene& Scene, const std::filesystem::path& FilePath,
                           FString* OutErrorMessage = nullptr);

    // 4주차 용 레거시 씬 형식 직렬화 함수입니다. 개선된 씬 형식이 도입되면 제거해주세요.
    static bool SerializeLegacy(const FScene& Scene, FString& OutJson,
                                FString* OutErrorMessage = nullptr);
};

class ENGINE_API FSceneDeserializer
{
  public:
    static std::unique_ptr<FScene> Deserialize(const FString& JsonSource,
                                               FString* OutErrorMessage = nullptr);
    static std::unique_ptr<FScene> LoadFromFile(const std::filesystem::path& FilePath,
                                                FString* OutErrorMessage = nullptr);
};
