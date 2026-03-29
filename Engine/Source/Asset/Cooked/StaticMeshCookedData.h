#pragma once

#include <memory>

#include "Asset/Cooked/CookedData.h"

namespace Asset
{
    struct FStaticMeshSectionData
    {
        FString MaterialSlotName;
        uint32  StartIndex = 0;
        uint32  IndexCount = 0;
    };

    enum class EStaticMeshVertexFormat : uint8
    {
        P,
        PN,
        PT,
        PNT,
        PC,
        PCT,
        PNC,
        PNCT
    };

    struct FStaticMeshCookedData : public FCookedData
    {
        EStaticMeshVertexFormat VertexFormat = EStaticMeshVertexFormat::P;

        TArray<uint8> VertexData;
        uint32        VertexStride = 0;
        uint32        VertexCount = 0;

        TArray<uint32>                 Indices;
        TArray<FStaticMeshSectionData> Sections;

        bool bHasNormals = false;
        bool bHasColors = false;
        bool bHasUVs = false;

        virtual EAssetType GetAssetType() const override { return EAssetType::StaticMesh; }

        virtual bool IsValid() const override
        {
            return !VertexData.empty() && VertexStride > 0 && VertexCount > 0 && !Indices.empty();
        }

        virtual void Reset() override
        {
            VertexFormat = EStaticMeshVertexFormat::P;
            VertexData.clear();
            VertexStride = 0;
            VertexCount = 0;
            Indices.clear();
            Sections.clear();
            bHasNormals = false;
            bHasColors = false;
            bHasUVs = false;
        }
    };

} // namespace Asset
