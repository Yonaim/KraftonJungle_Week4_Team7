#include "SpriteComponent.h"

#include "Asset/AssetManager.h"
#include "Asset/Texture2DAsset.h"
#include "Engine/Component/Core/ComponentProperty.h"
#include "SceneIO/SceneAssetPath.h"

#include "Core/Misc/BitMaskEnum.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/Types/RenderItem.h"
#include "Engine/Game/Actor.h"

namespace Engine::Component
{
    void USpriteComponent::SetTextureResource(FTextureResource* InTextureResource)
    {
        TextureResource = InTextureResource;
    }

    void USpriteComponent::SetTexturePath(const FString& InPath)
    {
        TexturePath = InPath;
        TextureResource = nullptr;
    }

    void USpriteComponent::SetBillboard(bool bInBillboard) { bBillboard = bInBillboard; }

    void USpriteComponent::SetBillboardOffset(const FVector& InBillboardOffset)
    {
        BillboardOffset = InBillboardOffset;
    }

    void USpriteComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        UQuadComponent::DescribeProperties(Builder);

        FComponentPropertyOptions TexturePathOptions;
        TexturePathOptions.ExpectedAssetPathKind = EComponentAssetPathKind::TextureImage;

        Builder.AddAssetPath(
            "texture_path", L"Texture Path", [this]() { return GetTexturePath(); },
            [this](const FString& InValue) { SetTexturePath(InValue); }, TexturePathOptions);

        Builder.AddBool(
            "billboard", L"Billboard", [this]() { return GetBillboard(); },
            [this](bool bInValue) { SetBillboard(bInValue); });
    }

    void USpriteComponent::ResolveAssetReferences(UAssetManager* InAssetManager)
    {
        TextureResource = nullptr;

        if (InAssetManager == nullptr || TexturePath.empty())
        {
            return;
        }

        const std::filesystem::path AbsolutePath =
            Engine::SceneIO::ResolveSceneAssetPathToAbsolute(TexturePath);
        if (AbsolutePath.empty())
        {
            UE_LOG(Asset, ELogVerbosity::Warning,
                   "Failed to resolve texture path for SpriteComponent: %s", TexturePath.c_str());
            return;
        }

        FAssetLoadParams LoadParams;
        LoadParams.ExplicitType = EAssetType::Texture;

        UAsset*          LoadedAsset = InAssetManager->Load(AbsolutePath.native(), LoadParams);
        UTexture2DAsset* TextureAsset = Cast<UTexture2DAsset>(LoadedAsset);
        if (TextureAsset == nullptr)
        {
            UE_LOG(Asset, ELogVerbosity::Warning,
                   "Failed to load texture asset for SpriteComponent: %s", TexturePath.c_str());
            return;
        }

        SetTextureResource(TextureAsset->GetResource());
    }

    void USpriteComponent::GetUVs(FVector2& OutUVMin, FVector2& OutUVMax) const
    {
        OutUVMin = FVector2(0.0f, 0.0f);
        OutUVMax = FVector2(1.0f, 1.0f);
    }

    void USpriteComponent::CollectRenderData(FSceneRenderData& OutRenderData,
                                             ESceneShowFlags   InShowFlags) const
    {
        if (!IsFlagSet(InShowFlags, ESceneShowFlags::SF_Sprites))
        {
            return;
        }

        AActor* Actor = GetOwnerActor();
        if (Actor == nullptr)
        {
            return;
        }

        FSpriteRenderItem SpriteItem = {};
        SpriteItem.TextureResource = GetTextureResource();
        SpriteItem.Color = GetColor();
        GetUVs(SpriteItem.UVMin, SpriteItem.UVMax);
        SpriteItem.Placement.Mode = GetBillboard() ? ERenderPlacementMode::WorldBillboard
                                                   : ERenderPlacementMode::World;
        SpriteItem.Placement.World = Actor->GetWorldMatrix();
        SpriteItem.Placement.WorldOffset = GetBillboardOffset();

        SpriteItem.State.ObjectId = Actor->GetObjectId();
        SpriteItem.State.bShowBounds = Actor->IsShowBounds();
        SpriteItem.State.SetVisible(Actor->IsVisible());
        SpriteItem.State.SetPickable(Actor->IsPickable());
        SpriteItem.State.SetSelected(Actor->IsSelected());
        SpriteItem.State.SetHovered(Actor->IsHovered());

        OutRenderData.Sprites.push_back(SpriteItem);
    }

    REGISTER_CLASS(Engine::Component, USpriteComponent)
} // namespace Engine::Component