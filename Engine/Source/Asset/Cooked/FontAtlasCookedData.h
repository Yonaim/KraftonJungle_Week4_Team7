#pragma once

#include <memory>

#include "Asset/Cooked/CookedData.h"
#include "Asset/Cooked/TextureCookedData.h"
#include "Asset/Core/FontTypes.h"

namespace Asset
{

    struct FFontAtlasCookedData : public FCookedData
    {
        std::shared_ptr<FTextureCookedData> AtlasTexture;
        FFontInfo                           Info;
        FFontCommon                         Common;
        TMap<uint32, FFontGlyph>            Glyphs;

        const FFontGlyph* FindGlyph(uint32 InCodePoint) const
        {
            auto It = Glyphs.find(InCodePoint);
            return It != Glyphs.end() ? &It->second : nullptr;
        }

        virtual EAssetType GetAssetType() const override { return EAssetType::FontAtlas; }

        virtual bool IsValid() const override
        {
            return AtlasTexture != nullptr && AtlasTexture->IsValid() && !Glyphs.empty();
        }

        virtual void Reset() override
        {
            AtlasTexture.reset();
            Info = {};
            Common = {};
            Glyphs.clear();
        }
    };

} // namespace Asset
