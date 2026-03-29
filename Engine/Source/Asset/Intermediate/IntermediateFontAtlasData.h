#pragma once

#include "Asset/Core/Types.h"
#include "Asset/Data/FontAtlasCookedData.h"

struct FIntermediateFontAtlasData
{
    FFontInfo                Info;
    FFontCommon              Common;
    TMap<uint32, FFontGlyph> Glyphs;
    FWString                 AtlasImagePath;
};
