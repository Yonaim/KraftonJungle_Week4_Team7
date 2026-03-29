#pragma once

#include "Asset/Core/AssetTags.h"
#include "Asset/Intermediate/IntermediateTextureData.h"
#include "Asset/Intermediate/IntermediateMaterialData.h"
#include "Asset/Intermediate/IntermediateStaticMeshData.h"
#include "Asset/Intermediate/IntermediateSubUVAtlasData.h"
#include "Asset/Intermediate/IntermediateFontAtlasData.h"
#include "Asset/Data/TextureCookedData.h"
#include "Asset/Data/MaterialCookedData.h"
#include "Asset/Data/StaticMeshCookedData.h"
#include "Asset/Data/SubUVAtlasCookedData.h"
#include "Asset/Data/FontAtlasCookedData.h"

namespace Asset
{

template <typename TTag> struct TAssetCacheTraits;

template <> struct TAssetCacheTraits<FTextureAssetTag>
{
    using IntermediateType = FIntermediateTextureData;
    using CookedType = FTextureCookedData;
};

template <> struct TAssetCacheTraits<FMaterialAssetTag>
{
    using IntermediateType = FIntermediateMaterialData;
    using CookedType = FMaterialCookedData;
};

template <> struct TAssetCacheTraits<FStaticMeshAssetTag>
{
    using IntermediateType = FIntermediateStaticMeshData;
    using CookedType = FStaticMeshCookedData;
};

template <> struct TAssetCacheTraits<FSubUVAtlasAssetTag>
{
    using IntermediateType = FIntermediateSubUVAtlasData;
    using CookedType = FSubUVAtlasCookedData;
};

template <> struct TAssetCacheTraits<FFontAtlasAssetTag>
{
    using IntermediateType = FIntermediateFontAtlasData;
    using CookedType = FFontAtlasCookedData;
};

template <typename TTag>
using TIntermediateTypeOf = typename TAssetCacheTraits<TTag>::IntermediateType;

template <typename TTag>
using TCookedTypeOf = typename TAssetCacheTraits<TTag>::CookedType;

} // namespace Asset
