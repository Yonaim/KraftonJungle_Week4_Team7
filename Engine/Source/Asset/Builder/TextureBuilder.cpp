#include "Asset/Builder/TextureBuilder.h"
#include "Asset/Cache/BuildSettings.h"

#include <cstring>

#include "Asset/Cache/DerivedKey.h"
#include "ThirdParty/stb/stb_image.h"

namespace Asset
{
    namespace
    {
        static std::string NarrowFromWide(const FWString& InPath)
        {
            if (InPath.empty())
            {
                return {};
            }

            const int RequiredSize =
                WideCharToMultiByte(CP_UTF8, 0, InPath.c_str(), -1, nullptr, 0, nullptr, nullptr);

            if (RequiredSize <= 1)
            {
                return {};
            }

            std::string Result;
            Result.resize(static_cast<size_t>(RequiredSize - 1));

            WideCharToMultiByte(CP_UTF8, 0, InPath.c_str(), -1, Result.data(), RequiredSize,
                                nullptr, nullptr);

            return Result;
        }
    } // namespace

    FTextureBuilder::FTextureBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

    std::shared_ptr<FTextureCookedData>
    FTextureBuilder::Build(const FWString& Path, const FTextureBuildSettings& Settings)
    {
        const FSourceRecord* Source = Cache.GetSource(Path);
        if (Source == nullptr)
        {
            return nullptr;
        }

        auto& IntermediateCache = Cache.GetIntermediateCache(FTextureAssetTag{});
        std::shared_ptr<FIntermediateTextureData> Intermediate =
            IntermediateCache.Find(Source->ContentHash);

        if (!Intermediate)
        {
            Intermediate = std::make_shared<FIntermediateTextureData>();
            if (!DecodeTexture(*Source, *Intermediate))
            {
                return nullptr;
            }

            IntermediateCache.Insert(Source->ContentHash, Intermediate);
        }

        const FDerivedKey DerivedKey = MakeDerivedKey(
            Source->ContentHash, Settings.bSRGB ? "TextureCook_sRGB" : "TextureCook_Linear");

        auto&                               CookedCache = Cache.GetCookedCache(FTextureAssetTag{});
        std::shared_ptr<FTextureCookedData> Cooked = CookedCache.Find(DerivedKey);

        if (!Cooked)
        {
            Cooked = CookTexture(*Source, *Intermediate, Settings);
            if (!Cooked)
            {
                return nullptr;
            }

            CookedCache.Insert(DerivedKey, Cooked);
        }

        return Cooked;
    }

    bool FTextureBuilder::DecodeTexture(const FSourceRecord&      Source,
                                        FIntermediateTextureData& OutData) const
    {
        const std::string Utf8Path = NarrowFromWide(Source.NormalizedPath);
        if (Utf8Path.empty())
        {
            return false;
        }

        int Width = 0;
        int Height = 0;
        int ChannelsInFile = 0;

        stbi_uc* DecodedPixels =
            stbi_load(Utf8Path.c_str(), &Width, &Height, &ChannelsInFile, STBI_rgb_alpha);

        if (DecodedPixels == nullptr)
        {
            return false;
        }

        if (Width <= 0 || Height <= 0)
        {
            stbi_image_free(DecodedPixels);
            return false;
        }

        const size_t PixelBytes = static_cast<size_t>(Width) * static_cast<size_t>(Height) * 4;

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
        Result->SourcePath = Source.NormalizedPath;
        Result->Width = Intermediate.Width;
        Result->Height = Intermediate.Height;
        Result->Channels = Intermediate.Channels;
        Result->Format = Intermediate.Format;
        Result->Pixels = Intermediate.Pixels;
        Result->bSRGB = Settings.bSRGB;

        return Result->IsValid() ? Result : nullptr;
    }

} // namespace Asset