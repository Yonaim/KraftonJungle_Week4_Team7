#pragma once

#include "Asset/Data/CookedData.h"

struct FTextureCookedData : public FCookedData
{
    FString       SourcePath;
    uint32        Width = 0;
    uint32        Height = 0;
    uint32        Channels = 0;
    bool          bSRGB = true;
    TArray<uint8> Pixels;

    virtual EAssetType GetAssetType() const override { return EAssetType::Texture; }

    virtual bool IsValid() const override { return Width > 0 && Height > 0 && !Pixels.empty(); }

    virtual void Reset() override
    {
        SourcePath.clear();
        Width = 0;
        Height = 0;
        Channels = 0;
        bSRGB = true;
        Pixels.clear();
    }
};
