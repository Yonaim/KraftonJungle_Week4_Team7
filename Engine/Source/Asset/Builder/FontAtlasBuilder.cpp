#include "Asset/Builder/FontAtlasBuilder.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "Asset/Cache/AssetKeyUtils.h"
#include "Asset/Cache/AssetBuildCache.h"

namespace Asset
{
    std::shared_ptr<FFontAtlasCookedData>
    FFontAtlasBuilder::Build(const std::filesystem::path& Path,
                             const FTextureBuildSettings& AtlasTextureSettings)
    {
        const FSourceRecord* Source = Cache.GetSource(FFontAtlasAssetTag{}, Path);
        if (Source == nullptr)
        {
            return nullptr;
        }

        auto& IntermediateCache = Cache.GetIntermediateCache(FFontAtlasAssetTag{});
        std::shared_ptr<FIntermediateFontAtlasData> Intermediate = ParseFontAtlas(*Source);
        if (!Intermediate)
        {
            return nullptr;
        }

        const FFontAtlasIntermediateKey IntermediateKey =
            KeyUtils::MakeIntermediateKey(*Intermediate);
        std::shared_ptr<FIntermediateFontAtlasData> CachedIntermediate =
            IntermediateCache.Find(IntermediateKey);
        if (CachedIntermediate)
        {
            Intermediate = CachedIntermediate;
        }
        else
        {
            IntermediateCache.Insert(IntermediateKey, Intermediate);
        }

        const FFontAtlasCookedKey CookedKey =
            KeyUtils::MakeCookedKey(IntermediateKey, AtlasTextureSettings);

        auto& CookedCache = Cache.GetCookedCache(FFontAtlasAssetTag{});
        std::shared_ptr<FFontAtlasCookedData> Cooked = CookedCache.Find(CookedKey);
        if (!Cooked)
        {
            Cooked = CookFontAtlas(*Source, *Intermediate, AtlasTextureSettings);
            if (!Cooked)
            {
                return nullptr;
            }
            CookedCache.Insert(CookedKey, Cooked);
        }

        return Cooked;
    }

    std::shared_ptr<FIntermediateFontAtlasData>
    FFontAtlasBuilder::ParseFontAtlas(const FSourceRecord& Source)
    {
        FString Text;
        if (!ReadAllText(Source.NormalizedPath, Text))
        {
            return nullptr;
        }

        auto Result = std::make_shared<FIntermediateFontAtlasData>();

        std::istringstream Stream(Text);
        FString            Line;
        while (std::getline(Stream, Line))
        {
            Line = Trim(Line);
            if (Line.empty() || Line[0] == '#')
            {
                continue;
            }

            std::istringstream Iss(Line);
            FString            Tag;
            Iss >> Tag;

            TMap<FString, FString> Pairs;
            TryParseKeyValueLine(Line, Pairs);

            if (Tag == "info")
            {
                auto It = Pairs.find("face");
                if (It != Pairs.end())
                {
                    Result->Info.Face = It->second;
                }
                It = Pairs.find("size");
                if (It != Pairs.end())
                {
                    Result->Info.Size = std::stoi(It->second);
                }
                It = Pairs.find("bold");
                if (It != Pairs.end())
                {
                    Result->Info.bBold = std::stoi(It->second) != 0;
                }
                It = Pairs.find("italic");
                if (It != Pairs.end())
                {
                    Result->Info.bItalic = std::stoi(It->second) != 0;
                }
                It = Pairs.find("unicode");
                if (It != Pairs.end())
                {
                    Result->Info.bUnicode = std::stoi(It->second) != 0;
                }
            }
            else if (Tag == "common")
            {
                auto It = Pairs.find("lineHeight");
                if (It != Pairs.end())
                {
                    Result->Common.LineHeight = static_cast<uint32>(std::stoul(It->second));
                }
                It = Pairs.find("base");
                if (It != Pairs.end())
                {
                    Result->Common.Base = static_cast<uint32>(std::stoul(It->second));
                }
                It = Pairs.find("scaleW");
                if (It != Pairs.end())
                {
                    Result->Common.ScaleW = static_cast<uint32>(std::stoul(It->second));
                }
                It = Pairs.find("scaleH");
                if (It != Pairs.end())
                {
                    Result->Common.ScaleH = static_cast<uint32>(std::stoul(It->second));
                }
                It = Pairs.find("pages");
                if (It != Pairs.end())
                {
                    Result->Common.Pages = static_cast<uint32>(std::stoul(It->second));
                }
                It = Pairs.find("packed");
                if (It != Pairs.end())
                {
                    Result->Common.bPacked = std::stoi(It->second) != 0;
                }
            }
            else if (Tag == "page")
            {
                auto It = Pairs.find("file");
                if (It != Pairs.end())
                {
                    Result->AtlasImagePath = ResolveRelativePath(Source.NormalizedPath, It->second);
                }
            }
            else if (Tag == "char")
            {
                FFontGlyph Glyph;
                auto       It = Pairs.find("id");
                if (It != Pairs.end())
                    Glyph.Id = static_cast<uint32>(std::stoul(It->second));
                It = Pairs.find("x");
                if (It != Pairs.end())
                    Glyph.X = static_cast<uint32>(std::stoul(It->second));
                It = Pairs.find("y");
                if (It != Pairs.end())
                    Glyph.Y = static_cast<uint32>(std::stoul(It->second));
                It = Pairs.find("width");
                if (It != Pairs.end())
                    Glyph.Width = static_cast<uint32>(std::stoul(It->second));
                It = Pairs.find("height");
                if (It != Pairs.end())
                    Glyph.Height = static_cast<uint32>(std::stoul(It->second));
                It = Pairs.find("xoffset");
                if (It != Pairs.end())
                    Glyph.XOffset = std::stoi(It->second);
                It = Pairs.find("yoffset");
                if (It != Pairs.end())
                    Glyph.YOffset = std::stoi(It->second);
                It = Pairs.find("xadvance");
                if (It != Pairs.end())
                    Glyph.XAdvance = std::stoi(It->second);
                It = Pairs.find("page");
                if (It != Pairs.end())
                    Glyph.Page = static_cast<uint32>(std::stoul(It->second));
                It = Pairs.find("chnl");
                if (It != Pairs.end())
                    Glyph.Channel = static_cast<uint32>(std::stoul(It->second));

                if (Glyph.Id != 0 || Glyph.IsValid())
                {
                    Result->Glyphs[Glyph.Id] = Glyph;
                }
            }
        }

        return (!Result->AtlasImagePath.empty() && !Result->Glyphs.empty()) ? Result : nullptr;
    }

    std::shared_ptr<FFontAtlasCookedData>
    FFontAtlasBuilder::CookFontAtlas(const FSourceRecord&              Source,
                                     const FIntermediateFontAtlasData& Intermediate,
                                     const FTextureBuildSettings&      AtlasTextureSettings)
    {
        FTextureBuilder                     TextureBuilder(Cache);
        std::shared_ptr<FTextureCookedData> AtlasTexture =
            TextureBuilder.Build(Intermediate.AtlasImagePath, AtlasTextureSettings);
        if (!AtlasTexture)
        {
            return nullptr;
        }

        auto Result = std::make_shared<FFontAtlasCookedData>();
        Result->SourcePath = std::filesystem::path(Source.NormalizedPath).string();
        Result->AtlasTexture = AtlasTexture;
        Result->Info = Intermediate.Info;
        Result->Common = Intermediate.Common;
        Result->Glyphs = Intermediate.Glyphs;
        return Result->IsValid() ? Result : nullptr;
    }

    bool FFontAtlasBuilder::ReadAllText(const std::filesystem::path& Path, FString& OutText)
    {
        std::ifstream File(Path, std::ios::in | std::ios::binary);
        if (!File)
        {
            return false;
        }

        std::ostringstream Buffer;
        Buffer << File.rdbuf();
        OutText = Buffer.str();
        return true;
    }

    FString FFontAtlasBuilder::Trim(const FString& Value)
    {
        const size_t Begin = Value.find_first_not_of(" \t\r\n");
        if (Begin == FString::npos)
        {
            return {};
        }

        const size_t End = Value.find_last_not_of(" \t\r\n");
        return Value.substr(Begin, End - Begin + 1);
    }

    FString FFontAtlasBuilder::ExtractQuotedValue(const FString& Line)
    {
        const size_t Begin = Line.find('"');
        if (Begin == FString::npos)
        {
            return {};
        }
        const size_t End = Line.find('"', Begin + 1);
        if (End == FString::npos || End <= Begin)
        {
            return {};
        }
        return Line.substr(Begin + 1, End - Begin - 1);
    }

    bool FFontAtlasBuilder::TryParseKeyValueLine(const FString&          Line,
                                                 TMap<FString, FString>& OutPairs)
    {
        std::istringstream Iss(Line);
        FString            Token;
        bool               bParsedAny = false;
        while (Iss >> Token)
        {
            const size_t EqualsPos = Token.find('=');
            if (EqualsPos == FString::npos)
            {
                continue;
            }

            FString Key = Token.substr(0, EqualsPos);
            FString Value = Token.substr(EqualsPos + 1);

            if (!Value.empty() && Value.front() == '"' && Value.back() != '"')
            {
                FString Tail;
                while (Iss >> Tail)
                {
                    Value += " " + Tail;
                    if (!Tail.empty() && Tail.back() == '"')
                    {
                        break;
                    }
                }
            }

            if (Value.size() >= 2 && Value.front() == '"' && Value.back() == '"')
            {
                Value = Value.substr(1, Value.size() - 2);
            }

            OutPairs[Key] = Value;
            bParsedAny = true;
        }
        return bParsedAny;
    }

    FWString FFontAtlasBuilder::ResolveRelativePath(const std::filesystem::path& BasePath,
                                                    const FString&               RelativePath)
    {
        const std::filesystem::path BaseDirectory = std::filesystem::path(BasePath).parent_path();
        return (BaseDirectory / std::filesystem::path(RelativePath)).lexically_normal();
    }

} // namespace Asset
