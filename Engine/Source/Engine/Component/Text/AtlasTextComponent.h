#pragma once

#include "TextRenderComponent.h"
#include "Renderer/RenderAsset/FontResource.h"

class AActor;

namespace Engine::Component
{
    class ENGINE_API UAtlasTextComponent : public UTextRenderComponent
    {
        DECLARE_RTTI(UAtlasTextComponent, UTextRenderComponent)

      public:
        UAtlasTextComponent() = default;
        ~UAtlasTextComponent() override = default;


        const FFontResource* GetFontResource() const { return FontResource; }
        FFontResource*       GetFontResource() { return FontResource; }
        void                 SetFontResource(FFontResource* InFontResource);
        const FString&       GetFontPath() const { return FontPath; }
        void                 SetFontPath(const FString& InFontPath);

        float GetTextScale() const { return TextScale; }
        void  SetTextScale(float InTextScale);

        float GetLetterSpacing() const { return LetterSpacing; }
        void  SetLetterSpacing(float InLetterSpacing);

        float GetLineSpacing() const { return LineSpacing; }
        void  SetLineSpacing(float InLineSpacing);
 
        void            DescribeProperties(FComponentPropertyBuilder& Builder) override;
        void            ResolveAssetReferences(UAssetManager* InAssetManager) override;
        virtual FMatrix GetRenderPlacementWorld(const AActor& InOwnerActor) const;
        virtual FVector GetRenderPlacementOffset(const AActor& InOwnerActor) const;

        EBasicMeshType GetBasicMeshType() const override;

      protected:
        Geometry::FAABB GetLocalAABB() const override { return {}; }

    
        FFontResource* FontResource = nullptr;
        FString        FontPath;

        float TextScale = 1.0f;
        float LetterSpacing = 0.0f;
        float LineSpacing = 0.0f;

    };
} // namespace Engine::Component
