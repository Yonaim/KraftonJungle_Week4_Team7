#pragma once

#include "Asset/Source/SourceCache.h"
#include "Asset/Cache/AssetCache.h"
#include "Asset/Cache/AssetCacheTraits.h"

namespace Asset
{

    // key = MakeDerivedKey
    class FAssetBuildCache
    {
      public:
        // ============================ Dispatcher =================================

        const FSourceRecord* GetSource(const FWString& Path)
        {
            const FSourceRecord* Source = SourceCache.GetOrLoad(Path);
            if (Source == nullptr)
            {
                return nullptr;
            }

            if (!SourceCache.EnsureContentHashLoaded(Path))
            {
                return nullptr;
            }

            return SourceCache.Find(Path);
        }

        void InvalidateSource(const FWString& Path) { SourceCache.Invalidate(Path); }

        void ClearAll()
        {
            SourceCache.Clear();

            IntermediateTextureCache.Clear();
            IntermediateMaterialCache.Clear();
            IntermediateStaticMeshCache.Clear();
            IntermediateSubUVAtlasCache.Clear();
            IntermediateFontAtlasCache.Clear();

            TextureCookedCache.Clear();
            MaterialCookedCache.Clear();
            StaticMeshCookedCache.Clear();
            SubUVAtlasCookedCache.Clear();
            FontAtlasCookedCache.Clear();
        }

        // ======================== GetCache() Dispatcher ==========================
        // dispatch by tag

        template <typename TTag>
        TIntermediateCache<TIntermediateTypeOf<TTag>>& GetIntermediateCache(TTag Tag)
        {
            return GetIntermediateCacheImpl(Tag);
        }

        template <typename TTag>
        const TIntermediateCache<TIntermediateTypeOf<TTag>>& GetIntermediateCache(TTag Tag) const
        {
            return GetIntermediateCacheImpl(Tag);
        }

        template <typename TTag> TCookedCache<TCookedTypeOf<TTag>>& GetCookedCache(TTag Tag)
        {
            return GetCookedCacheImpl(Tag);
        }

        template <typename TTag>
        const TCookedCache<TCookedTypeOf<TTag>>& GetCookedCache(TTag Tag) const
        {
            return GetCookedCacheImpl(Tag);
        }

      private:
        // ===================== Intermediate Cache Dispatcher =====================

        TIntermediateCache<FIntermediateTextureData>& GetIntermediateCacheImpl(FTextureAssetTag)
        {
            return IntermediateTextureCache;
        }
        const TIntermediateCache<FIntermediateTextureData>&
        GetIntermediateCacheImpl(FTextureAssetTag) const
        {
            return IntermediateTextureCache;
        }

        TIntermediateCache<FIntermediateMaterialData>& GetIntermediateCacheImpl(FMaterialAssetTag)
        {
            return IntermediateMaterialCache;
        }
        const TIntermediateCache<FIntermediateMaterialData>&
        GetIntermediateCacheImpl(FMaterialAssetTag) const
        {
            return IntermediateMaterialCache;
        }

        TIntermediateCache<FIntermediateStaticMeshData>&
        GetIntermediateCacheImpl(FStaticMeshAssetTag)
        {
            return IntermediateStaticMeshCache;
        }
        const TIntermediateCache<FIntermediateStaticMeshData>&
        GetIntermediateCacheImpl(FStaticMeshAssetTag) const
        {
            return IntermediateStaticMeshCache;
        }

        TIntermediateCache<FIntermediateSubUVAtlasData>&
        GetIntermediateCacheImpl(FSubUVAtlasAssetTag)
        {
            return IntermediateSubUVAtlasCache;
        }
        const TIntermediateCache<FIntermediateSubUVAtlasData>&
        GetIntermediateCacheImpl(FSubUVAtlasAssetTag) const
        {
            return IntermediateSubUVAtlasCache;
        }

        TIntermediateCache<FIntermediateFontAtlasData>& GetIntermediateCacheImpl(FFontAtlasAssetTag)
        {
            return IntermediateFontAtlasCache;
        }
        const TIntermediateCache<FIntermediateFontAtlasData>&
        GetIntermediateCacheImpl(FFontAtlasAssetTag) const
        {
            return IntermediateFontAtlasCache;
        }

        // ======================== Cooked Cache Dispatcher ========================

        TCookedCache<FTextureCookedData>& GetCookedCacheImpl(FTextureAssetTag)
        {
            return TextureCookedCache;
        }
        const TCookedCache<FTextureCookedData>& GetCookedCacheImpl(FTextureAssetTag) const
        {
            return TextureCookedCache;
        }

        TCookedCache<FMaterialCookedData>& GetCookedCacheImpl(FMaterialAssetTag)
        {
            return MaterialCookedCache;
        }
        const TCookedCache<FMaterialCookedData>& GetCookedCacheImpl(FMaterialAssetTag) const
        {
            return MaterialCookedCache;
        }

        TCookedCache<FStaticMeshCookedData>& GetCookedCacheImpl(FStaticMeshAssetTag)
        {
            return StaticMeshCookedCache;
        }
        const TCookedCache<FStaticMeshCookedData>& GetCookedCacheImpl(FStaticMeshAssetTag) const
        {
            return StaticMeshCookedCache;
        }

        TCookedCache<FSubUVAtlasCookedData>& GetCookedCacheImpl(FSubUVAtlasAssetTag)
        {
            return SubUVAtlasCookedCache;
        }
        const TCookedCache<FSubUVAtlasCookedData>& GetCookedCacheImpl(FSubUVAtlasAssetTag) const
        {
            return SubUVAtlasCookedCache;
        }

        TCookedCache<FFontAtlasCookedData>& GetCookedCacheImpl(FFontAtlasAssetTag)
        {
            return FontAtlasCookedCache;
        }
        const TCookedCache<FFontAtlasCookedData>& GetCookedCacheImpl(FFontAtlasAssetTag) const
        {
            return FontAtlasCookedCache;
        }

      private:
        // ============================== Source Cache =============================

        FSourceCache SourceCache;

        // =========================== Intermediate Cache ==========================

        TIntermediateCache<FIntermediateTextureData>    IntermediateTextureCache;
        TIntermediateCache<FIntermediateMaterialData>   IntermediateMaterialCache;
        TIntermediateCache<FIntermediateStaticMeshData> IntermediateStaticMeshCache;
        TIntermediateCache<FIntermediateSubUVAtlasData> IntermediateSubUVAtlasCache;
        TIntermediateCache<FIntermediateFontAtlasData>  IntermediateFontAtlasCache;

        // ============================== Cooked Cache =============================

        TCookedCache<FTextureCookedData>    TextureCookedCache;
        TCookedCache<FMaterialCookedData>   MaterialCookedCache;
        TCookedCache<FStaticMeshCookedData> StaticMeshCookedCache;
        TCookedCache<FSubUVAtlasCookedData> SubUVAtlasCookedCache;
        TCookedCache<FFontAtlasCookedData>  FontAtlasCookedCache;
    };

} // namespace Asset
