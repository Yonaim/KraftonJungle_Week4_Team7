#pragma once

#include "Asset/Core/FontTypes.h"

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
