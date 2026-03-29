#include "Asset/Builder/MaterialBuilder.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "Asset/Cache/DerivedKey.h"

namespace Asset
{
    namespace
    {
        static bool TryParseFloat3(std::istringstream& Iss, FVector& OutValue)
        {
            float X = 0.0f;
            float Y = 0.0f;
            float Z = 0.0f;
            if (!(Iss >> X >> Y >> Z))
            {
                return false;
            }

            OutValue = FVector(X, Y, Z);
            return true;
        }
    } // namespace

    std::shared_ptr<FMaterialCookedData> FMaterialBuilder::Build(const FWString& Path)
    {
        const FSourceRecord* Source = Cache.GetSource(Path);
        if (Source == nullptr)
        {
            return nullptr;
        }

        auto& IntermediateCache = Cache.GetIntermediateCache(FMaterialAssetTag{});
        std::shared_ptr<FIntermediateMaterialData> Intermediate =
            IntermediateCache.Find(Source->ContentHash);
        if (!Intermediate)
        {
            Intermediate = ParseMaterial(*Source);
            if (!Intermediate)
            {
                return nullptr;
            }
            IntermediateCache.Insert(Source->ContentHash, Intermediate);
        }

        const FDerivedKey DerivedKey = MakeDerivedKey(Source->ContentHash, "MaterialCook");

        auto& CookedCache = Cache.GetCookedCache(FMaterialAssetTag{});
        std::shared_ptr<FMaterialCookedData> Cooked = CookedCache.Find(DerivedKey);
        if (!Cooked)
        {
            Cooked = CookMaterial(*Source, *Intermediate);
            if (!Cooked)
            {
                return nullptr;
            }
            CookedCache.Insert(DerivedKey, Cooked);
        }

        return Cooked;
    }

    std::shared_ptr<FIntermediateMaterialData>
    FMaterialBuilder::ParseMaterial(const FSourceRecord& Source)
    {
        FString Text;
        if (!ReadAllText(Source.NormalizedPath, Text))
        {
            return nullptr;
        }

        auto Result = std::make_shared<FIntermediateMaterialData>();
        Result->Name = std::filesystem::path(Source.NormalizedPath).stem().string();

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

            if (Tag == "newmtl")
            {
                FString Name;
                Iss >> Name;
                if (!Name.empty())
                {
                    Result->Name = Name;
                }
            }
            else if (Tag == "Kd")
            {
                TryParseFloat3(Iss, Result->DiffuseColor);
            }
            else if (Tag == "Ka")
            {
                TryParseFloat3(Iss, Result->AmbientColor);
            }
            else if (Tag == "Ks")
            {
                TryParseFloat3(Iss, Result->SpecularColor);
            }
            else if (Tag == "Ns")
            {
                Iss >> Result->Shininess;
            }
            else if (Tag == "d")
            {
                Iss >> Result->Opacity;
            }
            else if (Tag == "Tr")
            {
                float Transparency = 0.0f;
                if (Iss >> Transparency)
                {
                    Result->Opacity = 1.0f - Transparency;
                }
            }
            else if (Tag == "map_Kd" || Tag == "map_Bump" || Tag == "bump" || Tag == "map_Ks" ||
                     Tag == "map_d" || Tag == "map_Ke")
            {
                FString TexturePath;
                std::getline(Iss, TexturePath);
                TexturePath = Trim(TexturePath);
                if (!TexturePath.empty())
                {
                    Result->TextureRefs.push_back({Tag, TexturePath});
                }
            }
        }

        return !Result->Name.empty() ? Result : nullptr;
    }

    std::shared_ptr<FMaterialCookedData>
    FMaterialBuilder::CookMaterial(const FSourceRecord&             Source,
                                   const FIntermediateMaterialData& Intermediate)
    {
        auto Result = std::make_shared<FMaterialCookedData>();
        Result->SourcePath = std::filesystem::path(Source.NormalizedPath).string();
        Result->Name = Intermediate.Name.empty()
                           ? std::filesystem::path(Source.NormalizedPath).stem().string()
                           : Intermediate.Name;
        Result->DiffuseColor = Intermediate.DiffuseColor;
        Result->AmbientColor = Intermediate.AmbientColor;
        Result->SpecularColor = Intermediate.SpecularColor;
        Result->Shininess = Intermediate.Shininess;
        Result->Opacity = Intermediate.Opacity;

        FTextureBuilder TextureBuilder(Cache);
        for (const FIntermediateMaterialTextureRef& TextureRef : Intermediate.TextureRefs)
        {
            const FWString ResolvedPath =
                ResolveRelativePath(Source.NormalizedPath, TextureRef.TexturePath);
            std::shared_ptr<FTextureCookedData> Texture = TextureBuilder.Build(ResolvedPath, {});
            if (!Texture)
            {
                continue;
            }

            FMaterialTextureBinding Binding;
            Binding.Slot = ResolveTextureSlot(TextureRef.SlotName);
            Binding.Texture = Texture;
            Result->TextureBindings.push_back(std::move(Binding));
        }

        return Result->IsValid() ? Result : nullptr;
    }

    bool FMaterialBuilder::ReadAllText(const FWString& Path, FString& OutText)
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

    FString FMaterialBuilder::Trim(const FString& Value)
    {
        const size_t Begin = Value.find_first_not_of(" \t\r\n");
        if (Begin == FString::npos)
        {
            return {};
        }

        const size_t End = Value.find_last_not_of(" \t\r\n");
        return Value.substr(Begin, End - Begin + 1);
    }

    EMaterialTextureSlot FMaterialBuilder::ResolveTextureSlot(const FString& SlotName)
    {
        if (SlotName == "map_Bump" || SlotName == "bump")
        {
            return EMaterialTextureSlot::Normal;
        }
        if (SlotName == "map_Ks")
        {
            return EMaterialTextureSlot::Specular;
        }
        if (SlotName == "map_d")
        {
            return EMaterialTextureSlot::Opacity;
        }
        if (SlotName == "map_Ke")
        {
            return EMaterialTextureSlot::Emissive;
        }
        return EMaterialTextureSlot::Diffuse;
    }

    FWString FMaterialBuilder::ResolveRelativePath(const FWString& BasePath,
                                                   const FString&  RelativePath)
    {
        const std::filesystem::path BaseDirectory = std::filesystem::path(BasePath).parent_path();
        return (BaseDirectory / std::filesystem::path(RelativePath)).lexically_normal().wstring();
    }

} // namespace Asset
