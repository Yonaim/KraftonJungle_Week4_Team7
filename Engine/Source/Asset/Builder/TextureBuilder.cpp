#include "Asset/Builder/TextureBuilder.h"

#include <cstring>
#include <filesystem>

#include "Asset/Cache/AssetKeyUtils.h"
#include "ThirdParty/stb/stb_image.h"

namespace Asset
{
    namespace
    {
        static std::string NarrowFromPath(const FWString& InPath)
        {
            if (InPath.empty())
            {
                return {};
            }

            return std::filesystem::path(InPath).string();
        }

        static FString StringFromPath(const FWString& InPath)
        {
            if (InPath.empty())
            {
                return {};
            }

            return std::filesystem::path(InPath).string();
        }
    } // namespace

    FTextureBuilder::FTextureBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

    std::shared_ptr<FTextureCookedData>
    FTextureBuilder::Build(const std::filesystem::path& Path, const FTextureBuildSettings& Settings)
    {
        LastBuildReport.Reset();
        const FSourceRecord* Source = Cache.GetSource(FTextureAssetTag{}, Path);
        if (Source == nullptr)
        {
            return nullptr;
        }

        auto& IntermediateCache = Cache.GetIntermediateCache(FTextureAssetTag{});

        std::shared_ptr<FIntermediateTextureData> Intermediate;
        FTextureIntermediateKey                   IntermediateKey;

        Intermediate = std::make_shared<FIntermediateTextureData>();
        if (!DecodeTexture(*Source, *Intermediate))
        {
            return nullptr;
        }

        IntermediateKey = KeyUtils::MakeIntermediateKey(*Intermediate);

        std::shared_ptr<FIntermediateTextureData> CachedIntermediate =
            IntermediateCache.Find(IntermediateKey);
        if (CachedIntermediate)
        {
            Intermediate = CachedIntermediate;
            LastBuildReport.bUsedCachedIntermediate = true;
        }
        else
        {
            IntermediateCache.Insert(IntermediateKey, Intermediate);
        }

        const FTextureCookedKey CookedKey = KeyUtils::MakeCookedKey(IntermediateKey, Settings);

        auto&                               CookedCache = Cache.GetCookedCache(FTextureAssetTag{});
        std::shared_ptr<FTextureCookedData> Cooked = CookedCache.Find(CookedKey);

        if (!Cooked)
        {
            LastBuildReport.bBuiltNewCooked = true;
            Cooked = CookTexture(*Source, *Intermediate, Settings);
            if (!Cooked)
            {
                return nullptr;
            }

            CookedCache.Insert(CookedKey, Cooked);
        }
        else
        {
            LastBuildReport.bUsedCachedCooked = true;
        }

        if (LastBuildReport.bUsedCachedCooked)
        {
            LastBuildReport.ResultSource = EAssetBuildResultSource::CookedCache;
        }
        else if (LastBuildReport.bUsedCachedIntermediate)
        {
            LastBuildReport.ResultSource = EAssetBuildResultSource::BuiltFromCachedIntermediate;
        }
        else if (Cooked)
        {
            LastBuildReport.ResultSource = EAssetBuildResultSource::BuiltFromFreshIntermediate;
        }

        return Cooked;
    }

    bool FTextureBuilder::DecodeTexture(const FSourceRecord&      Source,
                                        FIntermediateTextureData& OutData) const
    {
        const std::string PathString = NarrowFromPath(Source.NormalizedPath);
        if (PathString.empty())
        {
            return false;
        }

        int Width = 0;
        int Height = 0;
        int ChannelsInFile = 0;

        stbi_uc* DecodedPixels =
            stbi_load(PathString.c_str(), &Width, &Height, &ChannelsInFile, STBI_rgb_alpha);

        if (DecodedPixels == nullptr)
        {
            return false;
        }

        if (Width <= 0 || Height <= 0)
        {
            stbi_image_free(DecodedPixels);
            return false;
        }

        const size_t PixelBytes = static_cast<size_t>(Width) * static_cast<size_t>(Height) * 4u;

        OutData.Width = static_cast<uint32>(Width);
        OutData.Height = static_cast<uint32>(Height);
        OutData.Channels = 4;
        OutData.Format = EPixelFormat::RGBA8;
        OutData.Pixels.resize(PixelBytes);

        std::memcpy(OutData.Pixels.data(), DecodedPixels, PixelBytes);
        stbi_image_free(DecodedPixels);
        return true;
    }

    std::shared_ptr<FTextureCookedData>
    FTextureBuilder::CookTexture(const FSourceRecord&            Source,
                                 const FIntermediateTextureData& Intermediate,
                                 const FTextureBuildSettings&    Settings) const
    {
        if (Intermediate.Width == 0 || Intermediate.Height == 0)
        {
            return nullptr;
        }

        if (Intermediate.Pixels.empty())
        {
            return nullptr;
        }

        auto Result = std::make_shared<FTextureCookedData>();
        Result->SourcePath = StringFromPath(Source.NormalizedPath);
        Result->Width = Intermediate.Width;
        Result->Height = Intermediate.Height;
        Result->Channels = Intermediate.Channels;
        Result->Format = Intermediate.Format;
        Result->Pixels = Intermediate.Pixels;
        Result->bSRGB = Settings.bSRGB;

        return Result->IsValid() ? Result : nullptr;
    }

} // namespace Asset
