#include "SubUVComponent.h"

#include "Engine/Component/Core/ComponentProperty.h"
#include <algorithm>

#include "Asset/AssetManager.h"
#include "Asset/SubUVAtlasAsset.h"
#include "Engine/Component/Core/ComponentProperty.h"
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

    void USubUVComponent::GetUVs(FVector2& OutUVMin, FVector2& OutUVMax) const
    {
        OutUVMin = FVector2(0.0f, 0.0f);
        OutUVMax = FVector2(1.0f, 1.0f);

        if (SubUVResource != nullptr)
        {
            const int32 FrameCount = std::max(GetFrameCount(), 1);
            const int32 Index = std::clamp(GetFrameIndex(), 0, FrameCount - 1);
            const FSubUVFrame* Frame = SubUVResource->FindFrame(static_cast<uint32>(Index));

            const float AtlasWidth = static_cast<float>(
                std::max(SubUVResource->Common.ScaleW, SubUVResource->Atlas.Width));
            const float AtlasHeight = static_cast<float>(
                std::max(SubUVResource->Common.ScaleH, SubUVResource->Atlas.Height));

            if (Frame != nullptr && AtlasWidth > 0.0f && AtlasHeight > 0.0f)
            {
                OutUVMin = FVector2(static_cast<float>(Frame->X) / AtlasWidth,
                                    static_cast<float>(Frame->Y) / AtlasHeight);
                OutUVMax = FVector2(static_cast<float>(Frame->X + Frame->Width) / AtlasWidth,
                                    static_cast<float>(Frame->Y + Frame->Height) / AtlasHeight);
                return;
            }
        }

        const int32 Columns = std::max(GetAtlasColumns(), 1);
        const int32 Rows = std::max(GetAtlasRows(), 1);
        const int32 FrameCount = std::max(Columns * Rows, 1);
        const int32 Index = std::clamp(GetFrameIndex(), 0, FrameCount - 1);

        const int32 ColumnIndex = Index % Columns;
        const int32 RowIndex = Index / Columns;

        const float InvColumns = 1.0f / static_cast<float>(Columns);
        const float InvRows = 1.0f / static_cast<float>(Rows);

        OutUVMin = FVector2(ColumnIndex * InvColumns, RowIndex * InvRows);
        OutUVMax = FVector2((ColumnIndex + 1) * InvColumns, (RowIndex + 1) * InvRows);
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
