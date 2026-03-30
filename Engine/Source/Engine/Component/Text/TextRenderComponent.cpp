#include "TextRenderComponent.h"
#include "Engine/Component/Core/ComponentProperty.h"
#include "Engine/Game/Actor.h"
#include "SceneIO/SceneAssetPath.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/SceneView.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/Material/Material.h"
#include "Renderer/Types/RenderDebugColors.h"

#include <algorithm>

namespace
{
    constexpr float DefaultLineHeight = 16.0f;
    constexpr uint32 Utf8ReplacementCodePoint = static_cast<uint32>('?');

    TArray<uint32> DecodeUtf8CodePoints(const FString& InText)
    {
        TArray<uint32> CodePoints;
        CodePoints.reserve(InText.size());

        const uint8* Bytes = reinterpret_cast<const uint8*>(InText.data());
        const size_t ByteCount = InText.size();

        size_t Index = 0;
        while (Index < ByteCount)
        {
            const uint8 Lead = Bytes[Index];

            if (Lead <= 0x7F)
            {
                CodePoints.push_back(static_cast<uint32>(Lead));
                ++Index;
                continue;
            }

            uint32 CodePoint = Utf8ReplacementCodePoint;
            size_t SequenceLength = 1;

            auto IsContinuationByte =
                [Bytes, ByteCount](size_t InIndex)
            {
                return InIndex < ByteCount && (Bytes[InIndex] & 0xC0) == 0x80;
            };

            if ((Lead & 0xE0) == 0xC0)
            {
                SequenceLength = 2;
                if (IsContinuationByte(Index + 1))
                {
                    const uint32 Candidate =
                        ((static_cast<uint32>(Lead & 0x1F)) << 6) |
                        static_cast<uint32>(Bytes[Index + 1] & 0x3F);

                    if (Candidate >= 0x80)
                    {
                        CodePoint = Candidate;
                    }
                }
            }
            else if ((Lead & 0xF0) == 0xE0)
            {
                SequenceLength = 3;
                if (IsContinuationByte(Index + 1) && IsContinuationByte(Index + 2))
                {
                    const uint32 Candidate =
                        ((static_cast<uint32>(Lead & 0x0F)) << 12) |
                        ((static_cast<uint32>(Bytes[Index + 1] & 0x3F)) << 6) |
                        static_cast<uint32>(Bytes[Index + 2] & 0x3F);

                    if (Candidate >= 0x800 && !(Candidate >= 0xD800 && Candidate <= 0xDFFF))
                    {
                        CodePoint = Candidate;
                    }
                }
            }
            else if ((Lead & 0xF8) == 0xF0)
            {
                SequenceLength = 4;
                if (IsContinuationByte(Index + 1) && IsContinuationByte(Index + 2) &&
                    IsContinuationByte(Index + 3))
                {
                    const uint32 Candidate =
                        ((static_cast<uint32>(Lead & 0x07)) << 18) |
                        ((static_cast<uint32>(Bytes[Index + 1] & 0x3F)) << 12) |
                        ((static_cast<uint32>(Bytes[Index + 2] & 0x3F)) << 6) |
                        static_cast<uint32>(Bytes[Index + 3] & 0x3F);

                    if (Candidate >= 0x10000 && Candidate <= 0x10FFFF)
                    {
                        CodePoint = Candidate;
                    }
                }
            }

            CodePoints.push_back(CodePoint);
            Index += SequenceLength;
        }

        return CodePoints;
    }
}

namespace Engine::Component
{
    void UTextRenderComponent::SetText(const FString& InText) 
    { 
        if (Text != InText)
        {
            Text = InText; 
            MeshData = nullptr;
        }
    }

    void UTextRenderComponent::SetBillboard(bool bInBillboard) { bBillboard = bInBillboard; }

    void UTextRenderComponent::SetBillboardOffset(const FVector& InBillboardOffset)
    {
        BillboardOffset = InBillboardOffset;
    }

    void UTextRenderComponent::SetFontResource(FFontResource* InFontResource)
    {
        if (FontResource != InFontResource)
        {
            FontResource = InFontResource;
            MeshData = nullptr;
            Material = nullptr;
        }
    }

    void UTextRenderComponent::SetFontPath(const FString& InFontPath)
    {
        if (FontPath != InFontPath)
        {
            FontPath = InFontPath;
            FontResource = nullptr;
            MeshData = nullptr;
            Material = nullptr;
        }
    }

    void UTextRenderComponent::SetTextScale(float InTextScale) 
    { 
        if (TextScale != InTextScale)
        {
            TextScale = InTextScale; 
            MeshData = nullptr;
        }
    }

    void UTextRenderComponent::SetLetterSpacing(float InLetterSpacing)
    {
        if (LetterSpacing != InLetterSpacing)
        {
            LetterSpacing = InLetterSpacing;
            MeshData = nullptr;
        }
    }

    void UTextRenderComponent::SetLineSpacing(float InLineSpacing) 
    { 
        if (LineSpacing != InLineSpacing)
        {
            LineSpacing = InLineSpacing; 
            MeshData = nullptr;
        }
    }

    void UTextRenderComponent::SetLayoutMode(ETextLayoutMode InLayoutMode)
    {
        if (LayoutMode != InLayoutMode)
        {
            LayoutMode = InLayoutMode;
            MeshData = nullptr;
        }
    }

    void UTextRenderComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        UPrimitiveComponent::DescribeProperties(Builder);

        FComponentPropertyOptions FontPathOptions;
        FontPathOptions.ExpectedAssetPathKind = EComponentAssetPathKind::FontFile;

        Builder.AddString(
            "text", L"Text", [this]() { return GetText(); },
            [this](const FString& InValue) { SetText(InValue); });

        Builder.AddFloat(
            "text_scale", L"Text Scale", [this]() { return GetTextScale(); },
            [this](float InValue) { SetTextScale(InValue); });

        Builder.AddFloat(
            "letter_spacing", L"Letter Spacing", [this]() { return GetLetterSpacing(); },
            [this](float InValue) { SetLetterSpacing(InValue); });

        Builder.AddFloat(
            "line_spacing", L"Line Spacing", [this]() { return GetLineSpacing(); },
            [this](float InValue) { SetLineSpacing(InValue); });

        Builder.AddAssetPath(
            "font_path", L"Font Path", [this]() { return GetFontPath(); },
            [this](const FString& InValue) { SetFontPath(InValue); }, FontPathOptions);

        Builder.AddVector3(
            "billboard_offset", L"Billboard Offset", [this]() { return GetBillboardOffset(); },
            [this](const FVector& InValue) { SetBillboardOffset(InValue); });
            
        Builder.AddBool(
            "billboard", L"Billboard", [this]() { return GetBillboard(); },
            [this](bool bInValue) { SetBillboard(bInValue); });
    }

    void UTextRenderComponent::ResolveAssetReferences(UAssetManager* InAssetManager)
    {
        // FontResource = nullptr;
        //
        // if (InAssetManager == nullptr || FontPath.empty())
        // {
        //     return;
        // }
        //
        // const std::filesystem::path AbsolutePath =
        //     Engine::SceneIO::ResolveSceneAssetPathToAbsolute(FontPath);
        // if (AbsolutePath.empty())
        // {
        //     UE_LOG(Asset, ELogVerbosity::Warning,
        //            "Failed to resolve font path for TextRenderComponent: %s", FontPath.c_str());
        //     return;
        // }
        //
        // FAssetLoadParams LoadParams;
        // LoadParams.ExplicitType = EAssetType::Font;
        //
        // UAsset*     LoadedAsset = InAssetManager->Load(AbsolutePath.native(), LoadParams);
        // UFontAsset* FontAsset = Cast<UFontAsset>(LoadedAsset);
        // if (FontAsset == nullptr)
        // {
        //     UE_LOG(Asset, ELogVerbosity::Warning,
        //            "Failed to load font asset for TextRenderComponent: %s", FontPath.c_str());
        //     return;
        // }
        //
        // SetFontResource(&FontAsset->GetResource());
    }

    EBasicMeshType UTextRenderComponent::GetBasicMeshType() const { return EBasicMeshType::Quad; }

    FMatrix UTextRenderComponent::GetRenderPlacementWorld(const AActor& InOwnerActor) const
    {
        return InOwnerActor.GetWorldMatrix();
    }

    FVector UTextRenderComponent::GetRenderPlacementOffset(const AActor& InOwnerActor) const
    {
        (void)InOwnerActor;
        return FVector::ZeroVector;
    }

    UTextRenderComponent::FResolvedGlyph UTextRenderComponent::ResolveGlyph(const FFontResource& InFont, uint32 InCodePoint) const
    {
        if (const FFontGlyph* Glyph = InFont.FindGlyph(InCodePoint))
        {
            if (Glyph->IsValid())
            {
                return {Glyph, EResolvedGlyphKind::Normal};
            }
        }

        if (const FFontGlyph* QuestionGlyph = InFont.FindGlyph(static_cast<uint32>('?')))
        {
            if (QuestionGlyph->IsValid())
            {
                return {QuestionGlyph, EResolvedGlyphKind::QuestionFallback};
            }
        }

        return {nullptr, EResolvedGlyphKind::Missing};
    }

    float UTextRenderComponent::GetMissingGlyphAdvance(const FFontResource& InFont, float InLineHeight, float InScale) const
    {
        if (const FFontGlyph* SpaceGlyph = InFont.FindGlyph(static_cast<uint32>(' ')))
        {
            if (SpaceGlyph->IsValid())
            {
                return static_cast<float>(SpaceGlyph->XAdvance) * InScale;
            }
        }

        return InLineHeight * 0.5f * InScale;
    }

    UTextRenderComponent::FTextLayout UTextRenderComponent::BuildTextLayout() const
    {
        FTextLayout Layout;

        if (FontResource == nullptr || Text.empty())
        {
            return Layout;
        }

        const float RawLineHeight = (FontResource->Common.LineHeight > 0)
                                        ? static_cast<float>(FontResource->Common.LineHeight)
                                        : DefaultLineHeight;

        const float UnitScale =
            (RawLineHeight > 0.0f) ? (1.0f / RawLineHeight) : (1.0f / DefaultLineHeight);

        const float Scale = (TextScale > 0.0f) ? (TextScale * UnitScale) : UnitScale;
        const float LetterSpacingVal = LetterSpacing * UnitScale;
        const float LineSpacingVal = LineSpacing * UnitScale;

        const float MissingAdvance = GetMissingGlyphAdvance(*FontResource, RawLineHeight, Scale);
        const float MissingWidth = MissingAdvance;
        const float MissingHeight = RawLineHeight * Scale;

        float PenX = 0.0f;
        float PenY = 0.0f;
        bool  bHasBounds = false;

        const TArray<uint32> CodePoints = DecodeUtf8CodePoints(Text);

        for (uint32 CodePoint : CodePoints)
        {
            if (CodePoint == static_cast<uint32>('\r'))
            {
                continue;
            }

            if (CodePoint == static_cast<uint32>('\n'))
            {
                PenX = 0.0f;
                PenY += RawLineHeight * Scale + LineSpacingVal;
                continue;
            }

            if (CodePoint == static_cast<uint32>(' '))
            {
                float SpaceAdvance = 0.0f;

                if (const FFontGlyph* SpaceGlyph =
                        FontResource->FindGlyph(static_cast<uint32>(' ')))
                {
                    if (SpaceGlyph->IsValid())
                    {
                        SpaceAdvance = static_cast<float>(SpaceGlyph->XAdvance) * Scale;
                    }
                }

                if (SpaceAdvance <= 0.0f)
                {
                    SpaceAdvance = RawLineHeight * 0.25f * Scale;
                }

                PenX += SpaceAdvance;
                continue;
            }

            const FResolvedGlyph Resolved = ResolveGlyph(*FontResource, CodePoint);

            FLaidOutGlyph OutGlyph;

            if (Resolved.Kind == EResolvedGlyphKind::Missing)
            {
                OutGlyph.Glyph = nullptr;
                OutGlyph.bSolidColorQuad = true;
                OutGlyph.SolidColor = RenderDebugColors::MissingGlyph;

                OutGlyph.MinX = PenX;
                OutGlyph.MinY = PenY;
                OutGlyph.MaxX = PenX + MissingWidth;
                OutGlyph.MaxY = PenY + MissingHeight;

                PenX += MissingAdvance + LetterSpacingVal;
            }
            else
            {
                const FFontGlyph* Glyph = Resolved.Glyph;

                OutGlyph.Glyph = Glyph;
                OutGlyph.bSolidColorQuad = false;
                OutGlyph.MinX = PenX + static_cast<float>(Glyph->XOffset) * Scale;
                OutGlyph.MinY = PenY + static_cast<float>(Glyph->YOffset) * Scale;
                OutGlyph.MaxX = OutGlyph.MinX + static_cast<float>(Glyph->Width) * Scale;
                OutGlyph.MaxY = OutGlyph.MinY + static_cast<float>(Glyph->Height) * Scale;

                PenX += Scale * static_cast<float>(Glyph->XAdvance);
                PenX += LetterSpacingVal;
            }

            Layout.Glyphs.push_back(OutGlyph);

            if (!bHasBounds)
            {
                Layout.MinX = OutGlyph.MinX;
                Layout.MinY = OutGlyph.MinY;
                Layout.MaxX = OutGlyph.MaxX;
                Layout.MaxY = OutGlyph.MaxY;
                bHasBounds = true;
            }
            else
            {
                Layout.MinX = std::min(Layout.MinX, OutGlyph.MinX);
                Layout.MinY = std::min(Layout.MinY, OutGlyph.MinY);
                Layout.MaxX = std::max(Layout.MaxX, OutGlyph.MaxX);
                Layout.MaxY = std::max(Layout.MaxY, OutGlyph.MaxY);
            }
        }

        return Layout;
    }

    void UTextRenderComponent::CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const
    {
        if (Text.empty() || FontResource == nullptr)
        {
            return;
        }

        if (!IsFlagSet(InShowFlags, ESceneShowFlags::SF_BillboardText) && bBillboard)
        {
            return;
        }

        AActor* Actor = GetOwnerActor();
        if (Actor == nullptr)
        {
            return;
        }

        if (!MeshData)
        {
            MeshData = std::make_shared<FMeshData>();
            MeshData->Topology = EMeshTopology::EMT_TriangleList;

            FTextLayout Layout = BuildTextLayout();
            // if (Layout.IsValid())
            // {
            //     const float InvW = FontResource->GetInvAtlasWidth();
            //     const float InvH = FontResource->GetInvAtlasHeight();
            //     
            //     float CenterX = (Layout.MinX + Layout.MaxX) * 0.5f;
            //     float CenterY = (Layout.MinY + Layout.MaxY) * 0.5f;
            //
            //     for (const FLaidOutGlyph& G : Layout.Glyphs)
            //     {
            //         uint32 BaseIndex = static_cast<uint32>(MeshData->Vertices.size());
            //         
            //         float X0 = G.MinX - CenterX;
            //         float X1 = G.MaxX - CenterX;
            //         float Y0 = CenterY - G.MinY; 
            //         float Y1 = CenterY - G.MaxY;
            //
            //         FColor GlyphColor = G.bSolidColorQuad ? G.SolidColor : Color;
            //
            //         if (G.Glyph)
            //         {
            //             const float U0 = static_cast<float>(G.Glyph->X) * InvW;
            //             const float V0 = static_cast<float>(G.Glyph->Y) * InvH;
            //             const float U1 = static_cast<float>(G.Glyph->X + G.Glyph->Width) * InvW;
            //             const float V1 = static_cast<float>(G.Glyph->Y + G.Glyph->Height) * InvH;
            //
            //             // PaperSpriteComponent style: Local X is Vertical (Up), Local Y is Horizontal (Right)
            //             MeshData->Vertices.push_back({ FVector(Y1, X0, 0), GlyphColor, FVector(0,0,1), FVector2(U0, V1) }); // BL
            //             MeshData->Vertices.push_back({ FVector(Y1, X1, 0), GlyphColor, FVector(0,0,1), FVector2(U1, V1) }); // BR
            //             MeshData->Vertices.push_back({ FVector(Y0, X1, 0), GlyphColor, FVector(0,0,1), FVector2(U1, V0) }); // TR
            //             MeshData->Vertices.push_back({ FVector(Y0, X0, 0), GlyphColor, FVector(0,0,1), FVector2(U0, V0) }); // TL
            //         }
            //         else
            //         {
            //             // PaperSpriteComponent style: Local X is Vertical (Up), Local Y is Horizontal (Right)
            //             MeshData->Vertices.push_back({ FVector(Y1, X0, 0), GlyphColor, FVector(0,0,1), FVector2(0, 1) }); // BL
            //             MeshData->Vertices.push_back({ FVector(Y1, X1, 0), GlyphColor, FVector(0,0,1), FVector2(1, 1) }); // BR
            //             MeshData->Vertices.push_back({ FVector(Y0, X1, 0), GlyphColor, FVector(0,0,1), FVector2(1, 0) }); // TR
            //             MeshData->Vertices.push_back({ FVector(Y0, X0, 0), GlyphColor, FVector(0,0,1), FVector2(0, 0) }); // TL
            //         }
            //
            //         MeshData->Indices.push_back(BaseIndex + 0);
            //         MeshData->Indices.push_back(BaseIndex + 2);
            //         MeshData->Indices.push_back(BaseIndex + 1);
            //         MeshData->Indices.push_back(BaseIndex + 0);
            //         MeshData->Indices.push_back(BaseIndex + 3);
            //         MeshData->Indices.push_back(BaseIndex + 2);
            //     }
            //     MeshData->UpdateLocalBound();
            // }
        }

        if (!Material && FontResource)
        {
            Material = FGeneralRenderer::GetDefaultSpriteMaterial()->CreateDynamicMaterial();
            if (FontResource->GetSRV())
            {
                auto Tex = std::make_shared<FMaterialTexture>();
                Tex->TextureSRV = FontResource->GetSRV();
                Material->SetMaterialTexture(Tex);
            }
        }

        FRenderCommand Command;
        Command.MeshData = MeshData.get();
        Command.Material = Material ? Material.get() : FGeneralRenderer::GetDefaultSpriteMaterial();
        
        const FMatrix PlacementWorld = GetRenderPlacementWorld(*Actor);
        const FVector PlacementOffset = GetRenderPlacementOffset(*Actor);
        FVector Origin = PlacementWorld.GetOrigin() + PlacementOffset + BillboardOffset;

        if (bBillboard && OutRenderData.SceneView)
        {
            const FMatrix CameraWorld = OutRenderData.SceneView->GetViewMatrix().GetInverse();
            FVector RightAxis = CameraWorld.GetRightVector();
            FVector UpAxis = CameraWorld.GetUpVector();
            FVector ForwardAxis = CameraWorld.GetForwardVector();

            const FVector WorldScale = Actor->GetScale();
            FVector Row0 = UpAxis * WorldScale.X;
            FVector Row1 = RightAxis * WorldScale.Y;
            FVector Row2 = -ForwardAxis;

            Command.WorldMatrix.M[0][0] = Row0.X; Command.WorldMatrix.M[0][1] = Row0.Y; Command.WorldMatrix.M[0][2] = Row0.Z; Command.WorldMatrix.M[0][3] = 0.0f;
            Command.WorldMatrix.M[1][0] = Row1.X; Command.WorldMatrix.M[1][1] = Row1.Y; Command.WorldMatrix.M[1][2] = Row1.Z; Command.WorldMatrix.M[1][3] = 0.0f;
            Command.WorldMatrix.M[2][0] = Row2.X; Command.WorldMatrix.M[2][1] = Row2.Y; Command.WorldMatrix.M[2][2] = Row2.Z; Command.WorldMatrix.M[2][3] = 0.0f;
            Command.WorldMatrix.M[3][0] = Origin.X; Command.WorldMatrix.M[3][1] = Origin.Y; Command.WorldMatrix.M[3][2] = Origin.Z; Command.WorldMatrix.M[3][3] = 1.0f;
        }
        else
        {
            Command.WorldMatrix = PlacementWorld;
            Command.WorldMatrix.M[3][0] += PlacementOffset.X + BillboardOffset.X;
            Command.WorldMatrix.M[3][1] += PlacementOffset.Y + BillboardOffset.Y;
            Command.WorldMatrix.M[3][2] += PlacementOffset.Z + BillboardOffset.Z;
        }

        Command.ObjectId = Actor->GetObjectId();
        Command.bDrawAABB = Actor->IsSelected();
        Command.WorldAABB = GetWorldAABB();

        Command.bIsVisible = Actor->IsVisible();
        Command.bIsPickable = Actor->IsPickable();
        Command.bIsSelected = Actor->IsSelected();
        Command.bIsHovered = Actor->IsHovered();

        OutRenderData.RenderCommands.push_back(Command);
    }

    Geometry::FAABB UTextRenderComponent::GetLocalAABB() const
    {
        if (MeshData)
        {
            return Geometry::FAABB(MeshData->GetMinCoord(), MeshData->GetMaxCoord());
        }

        FTextLayout Layout = BuildTextLayout();
        if (Layout.IsValid())
        {
            float CenterX = (Layout.MinX + Layout.MaxX) * 0.5f;
            float CenterY = (Layout.MinY + Layout.MaxY) * 0.5f;
            return Geometry::FAABB(
                FVector(Layout.MinX - CenterX, CenterY - Layout.MaxY, 0.0f),
                FVector(Layout.MaxX - CenterX, CenterY - Layout.MinY, 0.0f)
            );
        }

        return {};
    }

    REGISTER_CLASS(Engine::Component, UTextRenderComponent)
} // namespace Engine::Component
