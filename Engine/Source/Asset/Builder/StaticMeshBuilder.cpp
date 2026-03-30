#include "Asset/Builder/StaticMeshBuilder.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "Asset/Cache/AssetKeyUtils.h"

namespace Asset
{

    namespace
    {
        struct FVertexP
        {
            FVector Position;
        };

        struct FVertexPN
        {
            FVector Position;
            FVector Normal;
        };

        struct FVertexPT
        {
            FVector  Position;
            FVector2 UV;
        };

        struct FVertexPNT
        {
            FVector  Position;
            FVector  Normal;
            FVector2 UV;
        };

        struct FVertexPC
        {
            FVector Position;
            FVector Color;
        };

        struct FVertexPCT
        {
            FVector  Position;
            FVector  Color;
            FVector2 UV;
        };

        struct FVertexPNC
        {
            FVector Position;
            FVector Normal;
            FVector Color;
        };

        struct FVertexPNCT
        {
            FVector  Position;
            FVector  Normal;
            FVector  Color;
            FVector2 UV;
        };

        struct FObjVertexKey
        {
            int32 PositionIndex = -1;
            int32 NormalIndex = -1;
            int32 UVIndex = -1;
            int32 ColorIndex = -1;

            bool operator==(const FObjVertexKey& Other) const
            {
                return PositionIndex == Other.PositionIndex && NormalIndex == Other.NormalIndex &&
                       UVIndex == Other.UVIndex && ColorIndex == Other.ColorIndex;
            }
        };

        struct FObjVertexKeyHasher
        {
            size_t operator()(const FObjVertexKey& Key) const
            {
                size_t Seed = 0;
                auto   HashCombine = [&](int32 Value)
                {
                    const size_t H = std::hash<int32>{}(Value);
                    Seed ^= H + 0x9e3779b9 + (Seed << 6) + (Seed >> 2);
                };

                HashCombine(Key.PositionIndex);
                HashCombine(Key.NormalIndex);
                HashCombine(Key.UVIndex);
                HashCombine(Key.ColorIndex);
                return Seed;
            }
        };

        static FString Trim(const FString& Value)
        {
            const size_t Begin = Value.find_first_not_of(" \t\r\n");
            if (Begin == FString::npos)
            {
                return {};
            }

            const size_t End = Value.find_last_not_of(" \t\r\n");
            return Value.substr(Begin, End - Begin + 1);
        }

        static int32 ResolveObjIndex(int32 RawIndex, size_t Count)
        {
            if (RawIndex > 0)
            {
                return RawIndex - 1;
            }
            if (RawIndex < 0)
            {
                return static_cast<int32>(Count) + RawIndex;
            }
            return -1;
        }

        static bool ParseFaceVertexToken(const FString&                     Token,
                                         FIntermediateMeshFaceVertex&       OutVertex,
                                         const FIntermediateStaticMeshData& MeshData)
        {
            std::stringstream Ss(Token);
            FString           Segment;
            int32             Segments[3] = {0, 0, 0};
            int32             SegmentIndex = 0;

            while (std::getline(Ss, Segment, '/') && SegmentIndex < 3)
            {
                if (!Segment.empty())
                {
                    Segments[SegmentIndex] = std::stoi(Segment);
                }
                ++SegmentIndex;
            }

            OutVertex.PositionIndex = ResolveObjIndex(Segments[0], MeshData.Positions.size());
            OutVertex.UVIndex = ResolveObjIndex(Segments[1], MeshData.UVs.size());
            OutVertex.NormalIndex = ResolveObjIndex(Segments[2], MeshData.Normals.size());

            return OutVertex.PositionIndex >= 0;
        }

        static void AppendVertexBytes(TArray<uint8>& Buffer, const void* Data, size_t SizeInBytes)
        {
            const size_t OldSize = Buffer.size();
            Buffer.resize(OldSize + SizeInBytes);
            std::memcpy(Buffer.data() + OldSize, Data, SizeInBytes);
        }

        static EStaticMeshVertexFormat ResolveVertexFormat(bool bHasNormals, bool bHasColors,
                                                           bool bHasUVs)
        {
            if (bHasNormals)
            {
                if (bHasColors)
                {
                    return bHasUVs ? EStaticMeshVertexFormat::PNCT : EStaticMeshVertexFormat::PNC;
                }
                return bHasUVs ? EStaticMeshVertexFormat::PNT : EStaticMeshVertexFormat::PN;
            }

            if (bHasColors)
            {
                return bHasUVs ? EStaticMeshVertexFormat::PCT : EStaticMeshVertexFormat::PC;
            }

            return bHasUVs ? EStaticMeshVertexFormat::PT : EStaticMeshVertexFormat::P;
        }

        static uint32 ResolveVertexStride(EStaticMeshVertexFormat Format)
        {
            switch (Format)
            {
            case EStaticMeshVertexFormat::P:
                return static_cast<uint32>(sizeof(FVertexP));
            case EStaticMeshVertexFormat::PN:
                return static_cast<uint32>(sizeof(FVertexPN));
            case EStaticMeshVertexFormat::PT:
                return static_cast<uint32>(sizeof(FVertexPT));
            case EStaticMeshVertexFormat::PNT:
                return static_cast<uint32>(sizeof(FVertexPNT));
            case EStaticMeshVertexFormat::PC:
                return static_cast<uint32>(sizeof(FVertexPC));
            case EStaticMeshVertexFormat::PCT:
                return static_cast<uint32>(sizeof(FVertexPCT));
            case EStaticMeshVertexFormat::PNC:
                return static_cast<uint32>(sizeof(FVertexPNC));
            case EStaticMeshVertexFormat::PNCT:
                return static_cast<uint32>(sizeof(FVertexPNCT));
            default:
                return 0;
            }
        }
    } // namespace

    std::shared_ptr<FStaticMeshCookedData>
    FStaticMeshBuilder::Build(const std::filesystem::path& Path,
                             const FStaticMeshBuildSettings& Settings)
    {
        const FSourceRecord* Source = Cache.GetSource(FStaticMeshAssetTag{}, Path);
        if (Source == nullptr)
        {
            return nullptr;
        }

        auto& IntermediateCache = Cache.GetIntermediateCache(FStaticMeshAssetTag{});
        std::shared_ptr<FIntermediateStaticMeshData> Intermediate = ParseObj(*Source);
        if (!Intermediate)
        {
            return nullptr;
        }

        const FStaticMeshIntermediateKey IntermediateKey = KeyUtils::MakeIntermediateKey(*Intermediate);
        std::shared_ptr<FIntermediateStaticMeshData> CachedIntermediate =
            IntermediateCache.Find(IntermediateKey);
        if (CachedIntermediate)
        {
            Intermediate = CachedIntermediate;
        }
        else
        {
            IntermediateCache.Insert(IntermediateKey, Intermediate);
        }

        const FStaticMeshCookedKey CookedKey = KeyUtils::MakeCookedKey(IntermediateKey, Settings);

        auto& CookedCache = Cache.GetCookedCache(FStaticMeshAssetTag{});
        std::shared_ptr<FStaticMeshCookedData> Cooked = CookedCache.Find(CookedKey);
        if (!Cooked)
        {
            Cooked = CookMesh(*Source, *Intermediate, Settings);
            if (!Cooked)
            {
                return nullptr;
            }
            CookedCache.Insert(CookedKey, Cooked);
        }

        return Cooked;
    }

    std::shared_ptr<FIntermediateStaticMeshData>
    FStaticMeshBuilder::ParseObj(const FSourceRecord& Source)
    {
        std::ifstream File(std::filesystem::path(Source.NormalizedPath));
        if (!File)
        {
            return nullptr;
        }

        auto Result = std::make_shared<FIntermediateStaticMeshData>();

        FString CurrentMaterialName;
        FString Line;
        while (std::getline(File, Line))
        {
            Line = Trim(Line);
            if (Line.empty() || Line[0] == '#')
            {
                continue;
            }

            std::istringstream Iss(Line);
            FString            Tag;
            Iss >> Tag;

            if (Tag == "v")
            {
                float X = 0.0f, Y = 0.0f, Z = 0.0f;
                Iss >> X >> Y >> Z;
                Result->Positions.emplace_back(X, Y, Z);

                float R = 0.0f, G = 0.0f, B = 0.0f;
                if (Iss >> R >> G >> B)
                {
                    Result->Colors.emplace_back(R, G, B);
                }
            }
            else if (Tag == "vt")
            {
                float U = 0.0f, V = 0.0f;
                Iss >> U >> V;
                Result->UVs.emplace_back(U, V);
            }
            else if (Tag == "vn")
            {
                float X = 0.0f, Y = 0.0f, Z = 0.0f;
                Iss >> X >> Y >> Z;
                Result->Normals.emplace_back(X, Y, Z);
            }
            else if (Tag == "usemtl")
            {
                Iss >> CurrentMaterialName;
            }
            else if (Tag == "f")
            {
                FIntermediateMeshFace Face;
                Face.MaterialName = CurrentMaterialName;

                FString VertexToken;
                while (Iss >> VertexToken)
                {
                    FIntermediateMeshFaceVertex FaceVertex;
                    if (ParseFaceVertexToken(VertexToken, FaceVertex, *Result))
                    {
                        Face.Vertices.push_back(FaceVertex);
                    }
                }

                if (Face.Vertices.size() >= 3)
                {
                    Result->Faces.push_back(std::move(Face));
                }
            }
        }

        if (Result->Positions.empty() || Result->Faces.empty())
        {
            return nullptr;
        }

        return Result;
    }

    std::shared_ptr<FStaticMeshCookedData>
    FStaticMeshBuilder::CookMesh(const FSourceRecord&               Source,
                                 const FIntermediateStaticMeshData& Intermediate,
                                 const FStaticMeshBuildSettings&    Settings)
    {
        auto Result = std::make_shared<FStaticMeshCookedData>();

        bool bHasColors = !Intermediate.Colors.empty() &&
                          Intermediate.Colors.size() == Intermediate.Positions.size();
        bool bHasNormals = !Intermediate.Normals.empty();
        bool bHasUVs = !Intermediate.UVs.empty();

        for (const FIntermediateMeshFace& Face : Intermediate.Faces)
        {
            for (const FIntermediateMeshFaceVertex& FaceVertex : Face.Vertices)
            {
                if (FaceVertex.NormalIndex < 0)
                {
                    bHasNormals = false;
                }
                if (FaceVertex.UVIndex < 0)
                {
                    bHasUVs = false;
                }
            }
        }

        if (Settings.bRecomputeNormals)
        {
            bHasNormals = false;
        }

        Result->bHasColors = bHasColors;
        Result->bHasNormals = bHasNormals;
        Result->bHasUVs = bHasUVs;
        Result->VertexFormat = ResolveVertexFormat(bHasNormals, bHasColors, bHasUVs);
        Result->VertexStride = ResolveVertexStride(Result->VertexFormat);

        std::unordered_map<FObjVertexKey, uint32, FObjVertexKeyHasher> VertexMap;

        FString                 ActiveMaterialName;
        FStaticMeshSectionData* CurrentSection = nullptr;

        auto EnsureSection = [&](const FString& MaterialName)
        {
            if (CurrentSection != nullptr && ActiveMaterialName == MaterialName)
            {
                return;
            }

            ActiveMaterialName = MaterialName;

            FStaticMeshSectionData Section;
            Section.MaterialSlotName = MaterialName.empty() ? "Default" : MaterialName;
            Section.StartIndex = static_cast<uint32>(Result->Indices.size());
            Section.IndexCount = 0;
            Result->Sections.push_back(Section);
            CurrentSection = &Result->Sections.back();
        };

        auto BuildKey = [&](const FIntermediateMeshFaceVertex& FaceVertex) -> FObjVertexKey
        {
            FObjVertexKey Key;
            Key.PositionIndex = FaceVertex.PositionIndex;
            Key.NormalIndex = bHasNormals ? FaceVertex.NormalIndex : -1;
            Key.UVIndex = bHasUVs ? FaceVertex.UVIndex : -1;
            Key.ColorIndex = bHasColors ? FaceVertex.PositionIndex : -1;
            return Key;
        };

        auto EmitVertex = [&](const FObjVertexKey& Key)
        {
            const FVector& Position = Intermediate.Positions[Key.PositionIndex];
            const FVector  Normal = (bHasNormals && Key.NormalIndex >= 0)
                                        ? Intermediate.Normals[Key.NormalIndex]
                                        : FVector(0.0f, 0.0f, 0.0f);
            FVector2       UV = (bHasUVs && Key.UVIndex >= 0) ? Intermediate.UVs[Key.UVIndex]
                                                              : FVector2(0.0f, 0.0f);
            const FVector  Color = (bHasColors && Key.ColorIndex >= 0)
                                       ? Intermediate.Colors[Key.ColorIndex]
                                       : FVector(0.0f, 0.0f, 0.0f);

            if (Settings.bFlipV && bHasUVs)
            {
                UV.Y = 1.0f - UV.Y;
            }

            switch (Result->VertexFormat)
            {
            case EStaticMeshVertexFormat::P:
            {
                const FVertexP Vertex = {Position};
                AppendVertexBytes(Result->VertexData, &Vertex, sizeof(Vertex));
                break;
            }
            case EStaticMeshVertexFormat::PN:
            {
                const FVertexPN Vertex = {Position, Normal};
                AppendVertexBytes(Result->VertexData, &Vertex, sizeof(Vertex));
                break;
            }
            case EStaticMeshVertexFormat::PT:
            {
                const FVertexPT Vertex = {Position, UV};
                AppendVertexBytes(Result->VertexData, &Vertex, sizeof(Vertex));
                break;
            }
            case EStaticMeshVertexFormat::PNT:
            {
                const FVertexPNT Vertex = {Position, Normal, UV};
                AppendVertexBytes(Result->VertexData, &Vertex, sizeof(Vertex));
                break;
            }
            case EStaticMeshVertexFormat::PC:
            {
                const FVertexPC Vertex = {Position, Color};
                AppendVertexBytes(Result->VertexData, &Vertex, sizeof(Vertex));
                break;
            }
            case EStaticMeshVertexFormat::PCT:
            {
                const FVertexPCT Vertex = {Position, Color, UV};
                AppendVertexBytes(Result->VertexData, &Vertex, sizeof(Vertex));
                break;
            }
            case EStaticMeshVertexFormat::PNC:
            {
                const FVertexPNC Vertex = {Position, Normal, Color};
                AppendVertexBytes(Result->VertexData, &Vertex, sizeof(Vertex));
                break;
            }
            case EStaticMeshVertexFormat::PNCT:
            {
                const FVertexPNCT Vertex = {Position, Normal, Color, UV};
                AppendVertexBytes(Result->VertexData, &Vertex, sizeof(Vertex));
                break;
            }
            default:
                break;
            }
        };

        auto GetOrCreateVertexIndex = [&](const FIntermediateMeshFaceVertex& FaceVertex) -> uint32
        {
            const FObjVertexKey Key = BuildKey(FaceVertex);
            auto                It = VertexMap.find(Key);
            if (It != VertexMap.end())
            {
                return It->second;
            }

            const uint32 NewIndex = Result->VertexCount;
            EmitVertex(Key);
            VertexMap.emplace(Key, NewIndex);
            ++Result->VertexCount;
            return NewIndex;
        };

        for (const FIntermediateMeshFace& Face : Intermediate.Faces)
        {
            EnsureSection(Face.MaterialName);

            const uint32 BaseIndex = static_cast<uint32>(Result->Indices.size());
            for (size_t VertexIndex = 1; VertexIndex + 1 < Face.Vertices.size(); ++VertexIndex)
            {
                Result->Indices.push_back(GetOrCreateVertexIndex(Face.Vertices[0]));
                Result->Indices.push_back(GetOrCreateVertexIndex(Face.Vertices[VertexIndex]));
                Result->Indices.push_back(GetOrCreateVertexIndex(Face.Vertices[VertexIndex + 1]));
            }

            if (CurrentSection != nullptr)
            {
                CurrentSection->IndexCount +=
                    static_cast<uint32>(Result->Indices.size() - BaseIndex);
            }
        }

        if (Result->Sections.empty())
        {
            FStaticMeshSectionData Section;
            Section.MaterialSlotName = "Default";
            Section.StartIndex = 0;
            Section.IndexCount = static_cast<uint32>(Result->Indices.size());
            Result->Sections.push_back(Section);
        }

        (void)Source;
        return Result->IsValid() ? Result : nullptr;
    }

} // namespace Asset
