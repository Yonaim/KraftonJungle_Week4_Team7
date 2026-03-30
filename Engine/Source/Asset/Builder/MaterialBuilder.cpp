#include "Asset/Builder/MaterialBuilder.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "Asset/Cache/AssetKeyUtils.h"

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

        static FIntermediateMaterialData* EnsureCurrentMaterial(
            FIntermediateMaterialLibraryData& Library, const FString& FallbackName)
        {
            if (Library.Materials.empty())
            {
                FIntermediateMaterialData Material;
                Material.Name = FallbackName;
                const uint32 MaterialIndex = static_cast<uint32>(Library.Materials.size());
                Library.Materials.push_back(std::move(Material));
                Library.NameToIndex[Library.Materials.back().Name] = MaterialIndex;
            }

            return &Library.Materials.back();
        }
    } // namespace

    std::shared_ptr<FMaterialCookedLibraryData>
    FMaterialBuilder::BuildLibrary(const std::filesystem::path& Path)
    {
        const FSourceRecord* Source = Cache.GetSource(FMaterialAssetTag{}, Path);
        if (Source == nullptr)
        {
            return nullptr;
        }

        auto& IntermediateCache = Cache.GetIntermediateCache(FMaterialAssetTag{});
        std::shared_ptr<FIntermediateMaterialLibraryData> Intermediate = ParseMaterialLibrary(*Source);
        if (!Intermediate)
        {
            return nullptr;
        }

        const FMaterialIntermediateKey IntermediateKey = KeyUtils::MakeIntermediateKey(*Intermediate);
        std::shared_ptr<FIntermediateMaterialLibraryData> CachedIntermediate =
            IntermediateCache.Find(IntermediateKey);
        if (CachedIntermediate)
        {
            Intermediate = CachedIntermediate;
        }
        else
        {
            IntermediateCache.Insert(IntermediateKey, Intermediate);
        }

        const FMaterialCookedKey CookedKey = KeyUtils::MakeCookedKey(IntermediateKey);

        auto& CookedCache = Cache.GetCookedCache(FMaterialAssetTag{});
        std::shared_ptr<FMaterialCookedLibraryData> Cooked = CookedCache.Find(CookedKey);
        if (!Cooked)
        {
            Cooked = CookMaterialLibrary(*Source, *Intermediate);
            if (!Cooked)
            {
                return nullptr;
            }
            CookedCache.Insert(CookedKey, Cooked);
        }

        return Cooked;
    }

    std::shared_ptr<FMaterialCookedData>
    FMaterialBuilder::BuildMaterial(const std::filesystem::path& Path, const FString& MaterialName)
    {
        std::shared_ptr<FMaterialCookedLibraryData> Library = BuildLibrary(Path);
        if (Library == nullptr || Library->Materials.empty())
        {
            return nullptr;
        }

        if (MaterialName.empty())
        {
            return std::make_shared<FMaterialCookedData>(Library->Materials.front());
        }

        const FMaterialCookedData* FoundMaterial = Library->FindMaterial(MaterialName);
        if (FoundMaterial == nullptr)
        {
            return nullptr;
        }

        return std::make_shared<FMaterialCookedData>(*FoundMaterial);
    }

    FString FMaterialBuilder::MakeMaterialAssetPath(const std::filesystem::path& LibraryPath,
                                                    const FString&               MaterialName)
    {
        if (MaterialName.empty())
        {
            return LibraryPath.generic_string();
        }

        return LibraryPath.generic_string() + "::" + MaterialName;
    }

    bool FMaterialBuilder::SplitMaterialAssetPath(const FString& InAssetPath, FString& OutLibraryPath,
                                                  FString& OutMaterialName)
    {
        OutLibraryPath = InAssetPath;
        OutMaterialName.clear();

        const size_t SeparatorPos = InAssetPath.rfind("::");
        if (SeparatorPos == FString::npos)
        {
            return false;
        }

        OutLibraryPath = InAssetPath.substr(0, SeparatorPos);
        OutMaterialName = InAssetPath.substr(SeparatorPos + 2);
        return true;
    }

    std::shared_ptr<FIntermediateMaterialLibraryData>
    FMaterialBuilder::ParseMaterialLibrary(const FSourceRecord& Source)
    {
        FString Text;
        if (!ReadAllText(Source.NormalizedPath, Text))
        {
            return nullptr;
        }

        auto Result = std::make_shared<FIntermediateMaterialLibraryData>();
        Result->SourcePath = std::filesystem::path(Source.NormalizedPath).generic_string();

        const FString DefaultName = std::filesystem::path(Source.NormalizedPath).stem().string();

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
                if (Name.empty())
                {
                    continue;
                }

                auto Existing = Result->NameToIndex.find(Name);
                if (Existing != Result->NameToIndex.end())
                {
                    continue;
                }

                FIntermediateMaterialData Material;
                Material.Name = Name;
                const uint32 MaterialIndex = static_cast<uint32>(Result->Materials.size());
                Result->Materials.push_back(std::move(Material));
                Result->NameToIndex[Name] = MaterialIndex;
                continue;
            }

            FIntermediateMaterialData* CurrentMaterial = EnsureCurrentMaterial(*Result, DefaultName);
            if (CurrentMaterial == nullptr)
            {
                continue;
            }

            if (Tag == "Kd")
            {
                TryParseFloat3(Iss, CurrentMaterial->DiffuseColor);
            }
            else if (Tag == "Ka")
            {
                TryParseFloat3(Iss, CurrentMaterial->AmbientColor);
            }
            else if (Tag == "Ks")
            {
                TryParseFloat3(Iss, CurrentMaterial->SpecularColor);
            }
            else if (Tag == "Ns")
            {
                Iss >> CurrentMaterial->Shininess;
            }
            else if (Tag == "d")
            {
                Iss >> CurrentMaterial->Opacity;
            }
            else if (Tag == "Tr")
            {
                float Transparency = 0.0f;
                if (Iss >> Transparency)
                {
                    CurrentMaterial->Opacity = 1.0f - Transparency;
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
                    CurrentMaterial->TextureRefs.push_back({Tag, TexturePath});
                }
            }
        }

        return Result->IsValid() ? Result : nullptr;
    }

    std::shared_ptr<FMaterialCookedLibraryData>
    FMaterialBuilder::CookMaterialLibrary(const FSourceRecord& Source,
                                          const FIntermediateMaterialLibraryData& Intermediate)
    {
        auto Result = std::make_shared<FMaterialCookedLibraryData>();
        Result->SourcePath = std::filesystem::path(Source.NormalizedPath).generic_string();

        for (const FIntermediateMaterialData& MaterialIntermediate : Intermediate.Materials)
        {
            FMaterialCookedData Material;
            Material.SourcePath = Result->SourcePath;
            Material.Name = MaterialIntermediate.Name.empty()
                                ? std::filesystem::path(Source.NormalizedPath).stem().string()
                                : MaterialIntermediate.Name;
            Material.DiffuseColor = MaterialIntermediate.DiffuseColor;
            Material.AmbientColor = MaterialIntermediate.AmbientColor;
            Material.SpecularColor = MaterialIntermediate.SpecularColor;
            Material.Shininess = MaterialIntermediate.Shininess;
            Material.Opacity = MaterialIntermediate.Opacity;

            for (const FIntermediateMaterialTextureRef& TextureRef : MaterialIntermediate.TextureRefs)
            {
                const EMaterialTextureSlot TextureSlot = ResolveTextureSlot(TextureRef.SlotName);
                const FWString             TexturePath = ResolveRelativePath(
                    std::filesystem::path(Source.NormalizedPath).parent_path(), TextureRef.TexturePath);
                if (TexturePath.empty())
                {
                    continue;
                }

                const FTextureBuildSettings TextureSettings{};
                FTextureBuilder             TextureBuilder(Cache);
                std::shared_ptr<FTextureCookedData> TextureCooked =
                    TextureBuilder.Build(TexturePath, TextureSettings);
                if (TextureCooked == nullptr || !TextureCooked->IsValid())
                {
                    continue;
                }

                Material.TextureBindings.push_back({TextureSlot, TextureCooked});
            }

            const uint32 MaterialIndex = static_cast<uint32>(Result->Materials.size());
            Result->NameToIndex[Material.Name] = MaterialIndex;
            Result->Materials.push_back(std::move(Material));
        }

        return Result->IsValid() ? Result : nullptr;
    }

    bool FMaterialBuilder::ReadAllText(const std::filesystem::path& Path, FString& OutText)
    {
        std::ifstream File(Path);
        if (!File)
        {
            return false;
        }

        std::ostringstream Oss;
        Oss << File.rdbuf();
        OutText = Oss.str();
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
        if (SlotName == "map_Kd")
        {
            return EMaterialTextureSlot::Diffuse;
        }
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

    FWString FMaterialBuilder::ResolveRelativePath(const std::filesystem::path& BasePath,
                                                   const FString&               RelativePath)
    {
        if (RelativePath.empty())
        {
            return {};
        }

        std::filesystem::path ResolvedPath(RelativePath);
        if (ResolvedPath.is_relative())
        {
            ResolvedPath = BasePath / ResolvedPath;
        }

        std::error_code ErrorCode;
        std::filesystem::path CanonicalPath = std::filesystem::weakly_canonical(ResolvedPath, ErrorCode);
        if (ErrorCode)
        {
            CanonicalPath = ResolvedPath.lexically_normal();
        }

        return CanonicalPath.native();
    }

} // namespace Asset
