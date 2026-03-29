#include "Asset/Builder/SubUVAtlasBuilder.h"

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

#include "Asset/Cache/DerivedKey.h"

namespace Asset
{
    namespace
    {
        static bool ExtractString(const FString& Text, const char* Key, FString& OutValue)
        {
            const std::regex Pattern("\\\"" + FString(Key) + "\\\"\\s*:\\s*\\\"([^\\\"]+)\\\"");
            std::smatch      Match;
            if (std::regex_search(Text, Match, Pattern) && Match.size() > 1)
            {
                OutValue = Match[1].str();
                return true;
            }
            return false;
        }

        static bool ExtractUInt(const FString& Text, const char* Key, uint32& OutValue)
        {
            const std::regex Pattern("\\\"" + FString(Key) + "\\\"\\s*:\\s*(\\d+)");
            std::smatch      Match;
            if (std::regex_search(Text, Match, Pattern) && Match.size() > 1)
            {
                OutValue = static_cast<uint32>(std::stoul(Match[1].str()));
                return true;
            }
            return false;
        }

        static bool ExtractFloat(const FString& Text, const char* Key, float& OutValue)
        {
            const std::regex Pattern("\\\"" + FString(Key) + "\\\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?)");
            std::smatch      Match;
            if (std::regex_search(Text, Match, Pattern) && Match.size() > 1)
            {
                OutValue = std::stof(Match[1].str());
                return true;
            }
            return false;
        }

        static bool ExtractBool(const FString& Text, const char* Key, bool& OutValue)
        {
            const std::regex Pattern("\\\"" + FString(Key) + "\\\"\\s*:\\s*(true|false|0|1)");
            std::smatch      Match;
            if (std::regex_search(Text, Match, Pattern) && Match.size() > 1)
            {
                const FString Value = Match[1].str();
                OutValue = Value == "true" || Value == "1";
                return true;
            }
            return false;
        }
    } // namespace

    std::shared_ptr<FSubUVAtlasCookedData>
    FSubUVAtlasBuilder::Build(const FWString&              Path,
                              const FTextureBuildSettings& AtlasTextureSettings)
    {
        const FSourceRecord* Source = Cache.GetSource(Path);
        if (Source == nullptr)
        {
            return nullptr;
        }

        auto& IntermediateCache = Cache.GetIntermediateCache(FSubUVAtlasAssetTag{});
        std::shared_ptr<FIntermediateSubUVAtlasData> Intermediate =
            IntermediateCache.Find(Source->ContentHash);
        if (!Intermediate)
        {
            Intermediate = ParseAtlas(*Source);
            if (!Intermediate)
            {
                return nullptr;
            }
            IntermediateCache.Insert(Source->ContentHash, Intermediate);
        }

        const FDerivedKey DerivedKey =
            MakeDerivedKey(Source->ContentHash, AtlasTextureSettings.ToKeyString());

        auto& CookedCache = Cache.GetCookedCache(FSubUVAtlasAssetTag{});
        std::shared_ptr<FSubUVAtlasCookedData> Cooked = CookedCache.Find(DerivedKey);
        if (!Cooked)
        {
            Cooked = CookAtlas(*Source, *Intermediate, AtlasTextureSettings);
            if (!Cooked)
            {
                return nullptr;
            }
            CookedCache.Insert(DerivedKey, Cooked);
        }

        return Cooked;
    }

    std::shared_ptr<FIntermediateSubUVAtlasData>
    FSubUVAtlasBuilder::ParseAtlas(const FSourceRecord& Source)
    {
        FString Text;
        if (!ReadAllText(Source.NormalizedPath, Text))
        {
            return nullptr;
        }

        auto Result = std::make_shared<FIntermediateSubUVAtlasData>();
        Result->Info.Name = std::filesystem::path(Source.NormalizedPath).stem().string();

        FString AtlasPath;
        if (ExtractString(Text, "image", AtlasPath) ||
            ExtractString(Text, "atlasImagePath", AtlasPath) ||
            ExtractString(Text, "texture", AtlasPath))
        {
            Result->AtlasImagePath = ResolveRelativePath(Source.NormalizedPath, AtlasPath);
        }

        ExtractString(Text, "name", Result->Info.Name);
        ExtractUInt(Text, "frameWidth", Result->Info.FrameWidth);
        ExtractUInt(Text, "frameHeight", Result->Info.FrameHeight);
        ExtractUInt(Text, "columns", Result->Info.Columns);
        ExtractUInt(Text, "rows", Result->Info.Rows);
        ExtractUInt(Text, "frameCount", Result->Info.FrameCount);
        ExtractFloat(Text, "fps", Result->Info.FPS);
        ExtractBool(Text, "loop", Result->Info.bLoop);

        const std::regex FramePattern(
            "\\{[^\\{\\}]*\\\"id\\\"\\s*:\\s*(\\d+)[^\\{\\}]*\\\"x\\\"\\s*:\\s*(\\d+)[^\\{\\}]*"
            "\\\"y\\\"\\s*:\\s*(\\d+)[^\\{\\}]*\\\"width\\\"\\s*:\\s*(\\d+)[^\\{\\}]*"
            "\\\"height\\\"\\s*:\\s*(\\d+)(?:[^\\{\\}]*\\\"pivotX\\\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?))"
            "?(?:[^\\{\\}]*\\\"pivotY\\\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?))?(?:[^\\{\\}]*"
            "\\\"duration\\\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?))?[^\\{\\}]*\\}");

        for (std::sregex_iterator It(Text.begin(), Text.end(), FramePattern), End; It != End; ++It)
        {
            FSubUVFrame Frame;
            Frame.Id = static_cast<uint32>(std::stoul((*It)[1].str()));
            Frame.X = static_cast<uint32>(std::stoul((*It)[2].str()));
            Frame.Y = static_cast<uint32>(std::stoul((*It)[3].str()));
            Frame.Width = static_cast<uint32>(std::stoul((*It)[4].str()));
            Frame.Height = static_cast<uint32>(std::stoul((*It)[5].str()));
            if ((*It)[6].matched)
                Frame.PivotX = std::stof((*It)[6].str());
            if ((*It)[7].matched)
                Frame.PivotY = std::stof((*It)[7].str());
            if ((*It)[8].matched)
                Frame.Duration = std::stof((*It)[8].str());
            Result->Frames.push_back(Frame);
        }

        const std::regex SequencePattern(
            "\\{[^\\{\\}]*\\\"name\\\"\\s*:\\s*\\\"([^\\\"]+)\\\"[^\\{\\}]*\\\"startFrame\\\"\\s*:"
            "\\s*(\\d+)[^\\{\\}]*\\\"endFrame\\\"\\s*:\\s*(\\d+)(?:[^\\{\\}]*\\\"loop\\\"\\s*:\\s*("
            "true|false|0|1))?[^\\{\\}]*\\}");
        for (std::sregex_iterator It(Text.begin(), Text.end(), SequencePattern), End; It != End;
             ++It)
        {
            FSubUVSequence Sequence;
            Sequence.Name = (*It)[1].str();
            Sequence.StartFrame = static_cast<uint32>(std::stoul((*It)[2].str()));
            Sequence.EndFrame = static_cast<uint32>(std::stoul((*It)[3].str()));
            if ((*It)[4].matched)
            {
                const FString LoopText = (*It)[4].str();
                Sequence.bLoop = LoopText == "true" || LoopText == "1";
            }
            Result->Sequences[Sequence.Name] = Sequence;
        }

        if (Result->Info.FrameCount == 0)
        {
            Result->Info.FrameCount = static_cast<uint32>(Result->Frames.size());
        }

        return (!Result->AtlasImagePath.empty() && !Result->Frames.empty()) ? Result : nullptr;
    }

    std::shared_ptr<FSubUVAtlasCookedData>
    FSubUVAtlasBuilder::CookAtlas(const FSourceRecord&               Source,
                                  const FIntermediateSubUVAtlasData& Intermediate,
                                  const FTextureBuildSettings&       AtlasTextureSettings)
    {
        FTextureBuilder                     TextureBuilder(Cache);
        std::shared_ptr<FTextureCookedData> AtlasTexture =
            TextureBuilder.Build(Intermediate.AtlasImagePath, AtlasTextureSettings);
        if (!AtlasTexture)
        {
            return nullptr;
        }

        auto Result = std::make_shared<FSubUVAtlasCookedData>();
        Result->SourcePath = std::filesystem::path(Source.NormalizedPath).string();
        Result->AtlasTexture = AtlasTexture;
        Result->Info = Intermediate.Info;
        Result->Frames = Intermediate.Frames;
        Result->Sequences = Intermediate.Sequences;
        return Result->IsValid() ? Result : nullptr;
    }

    bool FSubUVAtlasBuilder::ReadAllText(const FWString& Path, FString& OutText)
    {
        std::ifstream File(std::filesystem::path(Path), std::ios::in | std::ios::binary);
        if (!File)
        {
            return false;
        }

        std::ostringstream Buffer;
        Buffer << File.rdbuf();
        OutText = Buffer.str();
        return true;
    }

    FString FSubUVAtlasBuilder::Trim(const FString& Value)
    {
        const size_t Begin = Value.find_first_not_of(" \t\r\n");
        if (Begin == FString::npos)
        {
            return {};
        }

        const size_t End = Value.find_last_not_of(" \t\r\n");
        return Value.substr(Begin, End - Begin + 1);
    }

    FWString FSubUVAtlasBuilder::ResolveRelativePath(const FWString& BasePath,
                                                     const FString&  RelativePath)
    {
        const std::filesystem::path BaseDirectory = std::filesystem::path(BasePath).parent_path();
        return (BaseDirectory / std::filesystem::path(RelativePath)).lexically_normal().wstring();
    }

} // namespace Asset
