#pragma once

#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Renderer/Types/BasicMeshType.h"

namespace Engine::Component
{
    /** 선 하나에 대한 데이터 */
    struct FLineData
    {
        FVector Start;
        FVector End;
        FColor  Color;
        float   RemainingLifeTime;
    };

    /**
     * 디버그 선들을 배칭하여 그리기 위한 컴포넌트.
     * 렌더러에서 PrimitiveComponent로 수집되어 처리됩니다.
     */
    class ENGINE_API ULineBatchComponent : public UPrimitiveComponent
    {
        DECLARE_RTTI(ULineBatchComponent, UPrimitiveComponent)

    public:
        ULineBatchComponent();
        ~ULineBatchComponent() override = default;

        // UPrimitiveComponent 인터페이스 구현
        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::None; }

        void CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const override;
        
    protected:
        Geometry::FAABB GetLocalAABB() const override;

    public:
        /** 매 프레임 선들의 수명을 업데이트합니다. */
        void Update(float InDeltaTime) override;

        /** 선을 추가합니다. */
        void AddLine(const FVector& InStart, const FVector& InEnd, const FColor& InColor, float InLifeTime = 0.0f);

        /** 박스를 추가합니다. */
        void AddBox(const FVector& InMin, const FVector& InMax, const FColor& InColor, float InLifeTime = 0.0f);

        /** 구체를 추가합니다. (3개의 와이어 루프로 표현) */
        void AddSphere(const FVector& InCenter, float InRadius, int32 InSegments, const FColor& InColor, float InLifeTime = 0.0f);

        /** 현재 보관 중인 선 목록을 반환합니다. */
        const TArray<FLineData>& GetLines() const { return Lines; }

        /** 모든 선을 즉시 제거합니다. */
        void ClearLines();

    private:
        TArray<FLineData> Lines;
    };
} // namespace Engine::Component
