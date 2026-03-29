#pragma once

#include "Asset/Core/Types.h"
#include "Asset/Data/FontAtlasCookedData.h"

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
