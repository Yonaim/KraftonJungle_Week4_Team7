#pragma once

#include "Asset/Core/AssetCommonTypes.h"

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

} // namespace Asset
