#pragma once

#include <memory>

#include "Asset/Data/CookedData.h"

enum class EVertexFormat : uint8
{
    Unknown = 0,
    P,
    PT,
    PN,
    PNT,
    PNC,
    PNCT,
};

struct FStaticMeshSectionData
{
    FString MaterialSlotName;
    uint32  StartIndex = 0;
    uint32  IndexCount = 0;
};

struct FStaticMeshCookedData : public FCookedData
{
    EVertexFormat VertexFormat = EVertexFormat::Unknown;

    TArray<FVector>                Positions;
    TArray<FVector2>               Texcoords;
    TArray<FVector>                Normals;
    TArray<FVector4>               Colors;
    TArray<uint32>                 Indices;
    TArray<FStaticMeshSectionData> Sections;

    virtual EAssetType GetAssetType() const override { return EAssetType::StaticMesh; }

    virtual bool IsValid() const override
    {
        if (Positions.empty() || Indices.empty())
        {
            return false;
        }

        const size_t VertexCount = Positions.size();

        switch (VertexFormat)
        {
        case EVertexFormat::P:
            return true;

        case EVertexFormat::PT:
            return Texcoords.size() == VertexCount;

        case EVertexFormat::PN:
            return Normals.size() == VertexCount;

        case EVertexFormat::PNT:
            return Normals.size() == VertexCount && Texcoords.size() == VertexCount;

        case EVertexFormat::PNC:
            return Normals.size() == VertexCount && Colors.size() == VertexCount;

        case EVertexFormat::PNCT:
            return Normals.size() == VertexCount && Colors.size() == VertexCount &&
                   Texcoords.size() == VertexCount;

        default:
            return false;
        }
    }

    virtual void Reset() override
    {
        VertexFormat = EVertexFormat::Unknown;
        Positions.clear();
        Texcoords.clear();
        Normals.clear();
        Colors.clear();
        Indices.clear();
        Sections.clear();
    }
};