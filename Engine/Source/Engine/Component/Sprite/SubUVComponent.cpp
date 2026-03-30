#include "SubUVComponent.h"

#include "Engine/Component/Core/ComponentProperty.h"
#include <algorithm>

#include "Renderer/RenderAsset/SubUVAtlasResource.h"
#include "SceneIO/SceneAssetPath.h"

namespace Engine::Component
{
    void USubUVComponent::SetSubUVAtlasAsset(USubUVAtlas* InAsset)
    {
        SubUVAtlasAsset = InAsset;
        SetFrameIndex(FrameIndex);
    }

    const FSubUVAtlasRenderResource* USubUVComponent::GetSubUVAtlasRenderResource() const
    {
        return (SubUVAtlasAsset != nullptr && SubUVAtlasAsset->GetRenderResource())
                   ? SubUVAtlasAsset->GetRenderResource().get()
                   : nullptr;
    }

    FSubUVAtlasRenderResource* USubUVComponent::GetSubUVAtlasRenderResource()
    {
        return (SubUVAtlasAsset != nullptr && SubUVAtlasAsset->GetRenderResource())
                   ? SubUVAtlasAsset->GetRenderResource().get()
                   : nullptr;
    }

    void USubUVComponent::SetFrameIndex(int32 InFrameIndex)
    {
        FrameIndex = (InFrameIndex >= 0) ? InFrameIndex : 0;

        const int32 FrameCount = GetFrameCount();
        if (FrameCount > 0 && FrameIndex >= FrameCount)
        {
            FrameIndex = FrameCount - 1;
        }
    }

    int32 USubUVComponent::GetFrameCount() const
    {
        const FSubUVAtlasRenderResource* Resource = GetSubUVAtlasRenderResource();
        if (Resource != nullptr)
        {
            if (!Resource->Frames.empty())
            {
                return static_cast<int32>(Resource->Frames.size());
            }

            if (Resource->Info.FrameCount > 0)
            {
                return static_cast<int32>(Resource->Info.FrameCount);
            }
        }

        return AtlasRows * AtlasColumns;
    }

    void USubUVComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        UAtlasComponent::DescribeProperties(Builder);

        FComponentPropertyOptions IntOptions;
        IntOptions.DragSpeed = 1.0f;

        Builder.AddInt(
            "frame_index", L"Frame Index", [this]() { return GetFrameIndex(); },
            [this](int32 InValue) { SetFrameIndex(InValue); }, IntOptions);
    }

    REGISTER_CLASS(Engine::Component, USubUVComponent)
} // namespace Engine::Component
