#include "Core/CoreMinimal.h"
#include "TextRenderComponent.h"
#include "Engine/Component/Core/ComponentProperty.h"

namespace Engine::Component
{
    void UTextRenderComponent::SetText(const FString& InText) { Text = InText; }

    void UTextRenderComponent::SetBillboard(bool bInBillboard) { bBillboard = bInBillboard; }

    void UTextRenderComponent::SetBillboardOffset(const FVector& InBillboardOffset)
    {
        BillboardOffset = InBillboardOffset;
    }

    void UTextRenderComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        UPrimitiveComponent::DescribeProperties(Builder);

        Builder.AddString(
            "text", L"Text", [this]() { return GetText(); },
            [this](const FString& InValue) { SetText(InValue); });

        Builder.AddVector3(
            "billboard_offset", L"Billboard Offset", [this]() { return GetBillboardOffset(); },
            [this](const FVector& InValue) { SetBillboardOffset(InValue); });
    }
    EBasicMeshType UTextRenderComponent::GetBasicMeshType() const { return EBasicMeshType(); }
    REGISTER_CLASS(Engine::Component, UTextRenderComponent)
} // namespace Engine::Component