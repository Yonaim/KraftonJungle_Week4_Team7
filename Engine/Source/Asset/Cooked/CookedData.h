#pragma once

#include "Asset/Core/AssetDataBase.h"
#include "Core/Containers/String.h"

namespace Asset
{

    struct FCookedData : public IAssetData
    {
        virtual ~FCookedData() = default;

        FString SourcePath;
    };

} // namespace Asset
