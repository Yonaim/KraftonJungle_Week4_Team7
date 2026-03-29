#include "AtlasTextComponent.h"

#include "Engine/Component/Core/ComponentProperty.h"
#include "Engine/Game/Actor.h"

namespace Engine::Component
{
    void UAtlasTextComponent::SetFontAsset(UFontAtlas* InFontAsset) { FontAsset = InFontAsset; }

    const FFontAtlasRenderResource* UAtlasTextComponent::GetFontRenderResource() const
    {
        return (FontAsset != nullptr && FontAsset->GetRenderResource())
                   ? FontAsset->GetRenderResource().get()
                   : nullptr;
    }

    FFontAtlasRenderResource* UAtlasTextComponent::GetFontRenderResource()
    {
        return (FontAsset != nullptr && FontAsset->GetRenderResource())
                   ? FontAsset->GetRenderResource().get()
                   : nullptr;
    }

    void UAtlasTextComponent::SetTextScale(float InTextScale) { TextScale = InTextScale; }

    void UAtlasTextComponent::SetLetterSpacing(float InLetterSpacing)
    {
        LetterSpacing = InLetterSpacing;
    }

    void UAtlasTextComponent::SetLineSpacing(float InLineSpacing) { LineSpacing = InLineSpacing; }

    void UAtlasTextComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        UTextRenderComponent::DescribeProperties(Builder);

        Builder.AddFloat(
            "text_scale", L"Text Scale", [this]() { return GetTextScale(); },
            [this](float InValue) { SetTextScale(InValue); });
        Builder.AddFloat(
            "letter_spacing", L"Letter Spacing", [this]() { return GetLetterSpacing(); },
            [this](float InValue) { SetLetterSpacing(InValue); });
        Builder.AddFloat(
            "line_spacing", L"Line Spacing", [this]() { return GetLineSpacing(); },
            [this](float InValue) { SetLineSpacing(InValue); });
    }

    FMatrix UAtlasTextComponent::GetRenderPlacementWorld(const AActor& InOwnerActor) const
    {
        return InOwnerActor.GetWorldMatrix();
    }

    FVector UAtlasTextComponent::GetRenderPlacementOffset(const AActor& InOwnerActor) const
    {
        (void)InOwnerActor;
        return FVector::ZeroVector;
    }

    EBasicMeshType UAtlasTextComponent::GetBasicMeshType() const { return EBasicMeshType::None; }

    REGISTER_CLASS(Engine::Component, UAtlasTextComponent)
} // namespace Engine::Component
