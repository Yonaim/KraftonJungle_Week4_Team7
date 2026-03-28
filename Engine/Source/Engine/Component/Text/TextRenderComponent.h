#pragma once
#include "Engine/Component/Core/PrimitiveComponent.h"

namespace Engine::Component
{
    class ENGINE_API UTextRenderComponent : public Engine::Component::UPrimitiveComponent
    {
        DECLARE_RTTI(UTextRemderComponent, UPrimitiveComponent)

      public:
        UTextRenderComponent() = default;
        ~UTextRenderComponent() override = default;

        const FString& GetText() const { return Text; }
        void           SetText(const FString& InText);

        bool GetBillboard() const { return bBillboard; }
        void SetBillboard(bool bInBillboard);

        const FVector& GetBillboardOffset() const { return BillboardOffset; }
        void           SetBillboardOffset(const FVector& InBillboardOffset);

        void DescribeProperties(FComponentPropertyBuilder& Builder) override;

        EBasicMeshType GetBasicMeshType() const override;

      protected:
        Geometry::FAABB GetLocalAABB() const override { return {}; }

      protected:
        FString Text;

        bool    bBillboard = false;
        FVector BillboardOffset = FVector(0.0f, 0.0f, 0.0f);
    };
} // namespace Engine::Component