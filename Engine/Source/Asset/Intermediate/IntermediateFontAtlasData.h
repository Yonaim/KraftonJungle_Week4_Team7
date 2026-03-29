#pragma once

#include "Asset/Cooked/FontAtlasCookedData.h"

namespace Asset
{

    struct FIntermediateFontAtlasData
    {
        FFontInfo                Info;
        FFontCommon              Common;
        TMap<uint32, FFontGlyph> Glyphs;
        FWString                 AtlasImagePath;
    };

} // namespace Asset
