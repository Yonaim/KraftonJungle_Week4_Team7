#include "Asset/Builders/TextureBuilder.h"

#include <filesystem>

std::shared_ptr<FTextureCookedData> FTextureBuilder::Build(const FWString&              Path,
                                                           const FTextureBuildSettings& Settings)
{
    const FSourceRecord* Source = Cache.GetSource(Path);
    if (Source == nullptr)
    {
        return nullptr;
    }

    auto& IntermediateCache = Cache.GetIntermediateCache(FTextureAssetTag{});
    std::shared_ptr<FIntermediateTextureData> Intermediate =
        IntermediateCache.Find(FDerivedKey(Source->ContentHash));
    if (!Intermediate)
    {
        Intermediate = DecodeTexture(*Source);
        if (!Intermediate)
        {
            return nullptr;
        }
        IntermediateCache.Insert(FDerivedKey(Source->ContentHash), Intermediate);
    }

    const FDerivedKey DerivedKey = MakeDerivedKey(Source->ContentHash, Settings.ToKeyString());

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

std::shared_ptr<FIntermediateTextureData>
FTextureBuilder::DecodeTexture(const FSourceRecord& Source)
{
    auto Result = std::make_shared<FIntermediateTextureData>();
    Result->Channels = 4;
    Result->Format = EPixelFormat::RGBA8;

    // 실제 프로젝트에서는 여기서 stb_image / WIC / DirectXTex 같은 디코더를 연결하면 됨.
    // 지금 예시는 구조만 맞춘 껍데기라서 폭/높이는 채우지 않음.
    (void)Source;
    return Result;
}

std::shared_ptr<FTextureCookedData>
FTextureBuilder::CookTexture(const FSourceRecord&            Source,
                             const FIntermediateTextureData& Intermediate,
                             const FTextureBuildSettings&    Settings)
{
    auto Result = std::make_shared<FTextureCookedData>();
    Result->SourcePath = std::filesystem::path(Source.NormalizedPath).string();
    Result->Width = Intermediate.Width;
    Result->Height = Intermediate.Height;
    Result->Channels = Intermediate.Channels;
    Result->bSRGB = Settings.bSRGB;
    Result->Pixels = Intermediate.Pixels;
    return Result;
}
