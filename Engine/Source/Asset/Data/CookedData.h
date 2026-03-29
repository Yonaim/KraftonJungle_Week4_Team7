#pragma once

#include "Asset/Core/AssetDataBase.h"

namespace Asset
{

struct FCookedData : public IAssetData
{
    virtual ~FCookedData() = default;
};

} // namespace Asset
