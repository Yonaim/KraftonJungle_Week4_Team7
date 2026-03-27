#include "LineBatchComponent.h"
#include <algorithm>

namespace Engine::Component
{
    ULineBatchComponent::ULineBatchComponent()
    {
    }

    Geometry::FAABB ULineBatchComponent::GetLocalAABB() const
    {
        // 디버그용 선들이므로 컬링 등에서 제외되지 않도록 주의해야 함.
        // 일단 빈 박스를 반환합니다.
        return Geometry::FAABB();
    }

    void ULineBatchComponent::Update(float InDeltaTime)
    {
        UPrimitiveComponent::Update(InDeltaTime);

        // 선들의 수명 관리
        auto It = std::remove_if(Lines.begin(), Lines.end(), [InDeltaTime](FLineData& Line) {
            if (Line.RemainingLifeTime > 0.0f)
            {
                Line.RemainingLifeTime -= InDeltaTime;
                return Line.RemainingLifeTime <= 0.0f;
            }
            // LifeTime이 0.0f인 경우 한 프레임 출력 후 제거되도록 유도함.
            // AddLine 시점에 0.0f인 경우 바로 제거되는 것을 막기 위해 정책 결정 필요.
            // 여기서는 Update가 호출될 때 이미 그려진 뒤라면 제거하는 것으로 구현.
            return true;
        });

        if (It != Lines.end())
        {
            Lines.erase(It, Lines.end());
        }
    }

    void ULineBatchComponent::AddLine(const FVector& InStart, const FVector& InEnd, const FColor& InColor, float InLifeTime)
    {
        Lines.push_back({InStart, InEnd, InColor, InLifeTime});
    }

    void ULineBatchComponent::AddBox(const FVector& InMin, const FVector& InMax, const FColor& InColor, float InLifeTime)
    {
        const FVector P000(InMin.X, InMin.Y, InMin.Z);
        const FVector P100(InMax.X, InMin.Y, InMin.Z);
        const FVector P010(InMin.X, InMax.Y, InMin.Z);
        const FVector P110(InMax.X, InMax.Y, InMin.Z);
        const FVector P001(InMin.X, InMin.Y, InMax.Z);
        const FVector P101(InMax.X, InMin.Y, InMax.Z);
        const FVector P011(InMin.X, InMax.Y, InMax.Z);
        const FVector P111(InMax.X, InMax.Y, InMax.Z);

        AddLine(P000, P100, InColor, InLifeTime);
        AddLine(P100, P110, InColor, InLifeTime);
        AddLine(P110, P010, InColor, InLifeTime);
        AddLine(P010, P000, InColor, InLifeTime);

        AddLine(P001, P101, InColor, InLifeTime);
        AddLine(P101, P111, InColor, InLifeTime);
        AddLine(P111, P011, InColor, InLifeTime);
        AddLine(P011, P001, InColor, InLifeTime);

        AddLine(P000, P001, InColor, InLifeTime);
        AddLine(P100, P101, InColor, InLifeTime);
        AddLine(P110, P111, InColor, InLifeTime);
        AddLine(P010, P011, InColor, InLifeTime);
    }

    void ULineBatchComponent::AddSphere(const FVector& InCenter, float InRadius, int32 InSegments, const FColor& InColor, float InLifeTime)
    {
        if (InSegments < 3) InSegments = 16;
        const float Step = 2.0f * 3.1415926535f / static_cast<float>(InSegments);

        for (int32 i = 0; i < InSegments; ++i)
        {
            const float Angle = static_cast<float>(i) * Step;
            const float NextAngle = static_cast<float>(i + 1) * Step;

            const float CosA = std::cos(Angle);
            const float SinA = std::sin(Angle);
            const float CosNext = std::cos(NextAngle);
            const float SinNext = std::sin(NextAngle);

            // XY 루프
            AddLine(InCenter + FVector(CosA * InRadius, SinA * InRadius, 0.0f),
                    InCenter + FVector(CosNext * InRadius, SinNext * InRadius, 0.0f), InColor, InLifeTime);
            // YZ 루프
            AddLine(InCenter + FVector(0.0f, CosA * InRadius, SinA * InRadius),
                    InCenter + FVector(0.0f, CosNext * InRadius, SinNext * InRadius), InColor, InLifeTime);
            // ZX 루프
            AddLine(InCenter + FVector(SinA * InRadius, 0.0f, CosA * InRadius),
                    InCenter + FVector(SinNext * InRadius, 0.0f, CosNext * InRadius), InColor, InLifeTime);
        }
    }

    void ULineBatchComponent::ClearLines()
    {
        Lines.clear();
    }

    REGISTER_CLASS(Engine::Component, ULineBatchComponent)
} // namespace Engine::Component
