#include "PrimitiveComponent.h"
#include "Core/Logging/LogMacros.h"

#include "ComponentProperty.h"
#include "Core/Geometry/Primitives/AABBUtility.h"
#include "Core/Misc/BitMaskEnum.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/Types/RenderItem.h"
#include "Renderer/Types/BasicMeshType.h"
#include "Engine/Game/Actor.h"

#include <cfloat>

namespace Engine::Component
{
    const FColor& UPrimitiveComponent::GetColor() const { return Color; }

    void UPrimitiveComponent::SetColor(const FColor& NewColor) { Color = NewColor; }

    const Geometry::FAABB& UPrimitiveComponent::GetWorldAABB() const
    {
        if (bBoundsDirty)
        {
            const_cast<UPrimitiveComponent*>(this)->UpdateBounds();
            const_cast<UPrimitiveComponent*>(this)->bBoundsDirty = false;
        }

        return WorldAABB;
    }

    bool UPrimitiveComponent::GetWorldAABB(Geometry::FAABB& OutWorldAABB) const
    {
        OutWorldAABB = GetWorldAABB();
        return true;
    }

    bool UPrimitiveComponent::GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const
    {
        return false;
    }

    void UPrimitiveComponent::Update(float DeltaTime)
    {
        USceneComponent::Update(DeltaTime);

        if (bBoundsDirty)
        {
            //UE_LOG(UPrimitiveComponent, ELogVerbosity::Log, "Bounds Update!");
            UpdateBounds();
            bBoundsDirty = false;
        }
    }

    void UPrimitiveComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        USceneComponent::DescribeProperties(Builder);
        Builder.AddColor(
            "color", L"Color", [this]() { return GetColor(); },
            [this](const FColor& InColor) { SetColor(InColor); });
    }

    void UPrimitiveComponent::CollectRenderData(FSceneRenderData& OutRenderData,
                                                ESceneShowFlags   InShowFlags) const
    {
        if (!IsFlagSet(InShowFlags, ESceneShowFlags::SF_Primitives))
        {
            return;
        }

        if (GetBasicMeshType() == EBasicMeshType::None)
        {
            return;
        }

        AActor* Actor = GetOwnerActor();
        if (Actor == nullptr)
        {
            return;
        }

        FPrimitiveRenderItem PrimitiveItem = {};
        PrimitiveItem.World = GetRelativeMatrix();
        PrimitiveItem.Color = Actor->GetColor();
        PrimitiveItem.MeshType = GetBasicMeshType();
        PrimitiveItem.WorldAABB = GetWorldAABB();
        PrimitiveItem.bHasWorldAABB = true;

        PrimitiveItem.State.ObjectId = Actor->GetObjectId();
        PrimitiveItem.State.bShowBounds = Actor->IsShowBounds();
        PrimitiveItem.State.SetVisible(Actor->IsVisible());
        PrimitiveItem.State.SetPickable(Actor->IsPickable());
        PrimitiveItem.State.SetSelected(Actor->IsSelected());
        PrimitiveItem.State.SetHovered(Actor->IsHovered());

        OutRenderData.Primitives.push_back(PrimitiveItem);
    }

    void UPrimitiveComponent::UpdateBounds()
    {
        const FMatrix WorldMatrix = GetRelativeMatrix();
        TArray<Geometry::FTriangle> LocalTriangles;

        if (GetLocalTriangles(LocalTriangles) && !LocalTriangles.empty())
        {
            FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
            FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

            auto Expand = [&Min, &Max](const FVector& P)
            {
                Min.X = std::min(Min.X, P.X);
                Min.Y = std::min(Min.Y, P.Y);
                Min.Z = std::min(Min.Z, P.Z);

                Max.X = std::max(Max.X, P.X);
                Max.Y = std::max(Max.Y, P.Y);
                Max.Z = std::max(Max.Z, P.Z);
            };

            for (const Geometry::FTriangle& Triangle : LocalTriangles)
            {
                Expand(WorldMatrix.TransformPosition(Triangle.V0));
                Expand(WorldMatrix.TransformPosition(Triangle.V1));
                Expand(WorldMatrix.TransformPosition(Triangle.V2));
            }

            WorldAABB = Geometry::FAABB(Min, Max);
            return;
        }

        WorldAABB = Geometry::TransformAABB(GetLocalAABB(), WorldMatrix);
    }

    void UPrimitiveComponent::OnTransformChanged() { bBoundsDirty = true; }

} // namespace Engine::Component
