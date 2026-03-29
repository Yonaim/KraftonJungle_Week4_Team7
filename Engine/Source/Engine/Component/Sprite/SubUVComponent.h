#pragma once

#include "Asset/SubUVAtlas.h"
#include "AtlasComponent.h"
#include "Core/CoreMinimal.h"

namespace Engine::Component
{
    class ENGINE_API USubUVComponent : public UAtlasComponent
    {
        DECLARE_RTTI(USubUVComponent, UAtlasComponent)

      public:
        USubUVComponent() = default;
        ~USubUVComponent() override = default;

        const USubUVAtlas* GetSubUVAtlasAsset() const { return SubUVAtlasAsset; }
        USubUVAtlas*       GetSubUVAtlasAsset() { return SubUVAtlasAsset; }
        void               SetSubUVAtlasAsset(USubUVAtlas* InAsset);

        const FSubUVAtlasRenderResource* GetSubUVAtlasRenderResource() const;
        FSubUVAtlasRenderResource*       GetSubUVAtlasRenderResource();

        int32 GetFrameIndex() const { return FrameIndex; }
        void  SetFrameIndex(int32 InFrameIndex);

        int32 GetFrameCount() const;

        void DescribeProperties(FComponentPropertyBuilder& Builder) override;

      protected:
        USubUVAtlas* SubUVAtlasAsset = nullptr;
        int32        FrameIndex = 0;
    };
} // namespace Engine::Component
