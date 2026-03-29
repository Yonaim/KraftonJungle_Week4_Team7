#pragma once

#include <memory>

#include "Asset/Cooked/CookedData.h"
#include "Asset/Cooked/TextureCookedData.h"
#include "Asset/Core/SubUVTypes.h"

namespace Asset
{

    struct FSubUVAtlasCookedData : public FCookedData
    {
        std::shared_ptr<FTextureCookedData> AtlasTexture;
        FSubUVAtlasInfo                     Info;
        TArray<FSubUVFrame>                 Frames;
        TMap<FString, FSubUVSequence>       Sequences;

        const FSubUVFrame* FindFrame(uint32 InId) const
        {
            for (const FSubUVFrame& Frame : Frames)
            {
                if (Frame.Id == InId)
                {
                    return &Frame;
                }
            }
            return nullptr;
        }

        const FSubUVSequence* FindSequence(const FString& InName) const
        {
            auto It = Sequences.find(InName);
            return It != Sequences.end() ? &It->second : nullptr;
        }

        virtual EAssetType GetAssetType() const override { return EAssetType::SubUVAtlas; }

        virtual bool IsValid() const override
        {
            return AtlasTexture != nullptr && AtlasTexture->IsValid() && !Frames.empty();
        }

        virtual void Reset() override
        {
            AtlasTexture.reset();
            Info = {};
            Frames.clear();
            Sequences.clear();
        }
    };

} // namespace Asset
