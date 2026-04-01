#pragma once

#include <filesystem>
#include "Engine/Asset/Texture.h"
#include "Engine/Component/Mesh/MeshComponent.h"
#include "Renderer/Types/BasicMeshType.h"

class UStaticMesh;
struct FMeshData;
class UMaterial;

namespace Engine::Component
{
    class ENGINE_API UPaperSpriteComponent : public UMeshComponent
    {
        DECLARE_RTTI(UPaperSpriteComponent, UMeshComponent)

      public:
        UPaperSpriteComponent() = default;
        ~UPaperSpriteComponent() override = default;

        static const std::filesystem::path& GetDefaultQuadMeshPath();

        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::Quad; }

        const std::filesystem::path& GetMeshAssetPath() const { return MeshPath; }
        void                         SetMeshAssetPath(const std::filesystem::path& InPath);
        void                         SetMeshAssetPath(const FString& InPath)
        {
            SetMeshAssetPath(std::filesystem::path(InPath));
        }

        const UStaticMesh* GetMeshAsset() const { return MeshAsset; }
        UStaticMesh*       GetMeshAsset() { return MeshAsset; }
        void               SetMeshAsset(UStaticMesh* InMeshAsset);

        const UTexture* GetTextureAsset() const { return TextureAsset; }
        UTexture*       GetTextureAsset() { return TextureAsset; }
        void            SetTextureAsset(UTexture* InTextureAsset);

        const std::filesystem::path& GetTexturePath() const { return TexturePath; }
        void                         SetTexturePath(const std::filesystem::path& InPath);
        void                         SetTexturePath(const FString& InPath)
        {
            SetTexturePath(std::filesystem::path(InPath));
        }

        const FTextureRenderResource* GetTextureRenderResource() const;
        FTextureRenderResource*       GetTextureRenderResource();
        const FTextureRenderResource* GetTextureResource() const
        {
            return GetTextureRenderResource();
        }
        FTextureRenderResource* GetTextureResource() { return GetTextureRenderResource(); }

        bool GetBillboard() const { return bBillboard; }
        void SetBillboard(bool bInBillboard);

        const FVector& GetBillboardOffset() const { return BillboardOffset; }
        void           SetBillboardOffset(const FVector& InBillboardOffset);

        void DescribeProperties(FComponentPropertyBuilder& Builder) override;

        void CollectRenderData(FSceneRenderData& OutRenderData,
                               ESceneShowFlags   InShowFlags) const override;

        bool GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const override;

      protected:
        Geometry::FAABB GetLocalAABB() const override;
        void            UpdateBounds() override;

      protected:
        virtual void GetUVs(FVector2& OutUVMin, FVector2& OutUVMax) const;

      private:
        FVector2 GetSpriteAspectScale() const;
        void     EnsureDynamicQuadMeshData() const;
        void     EnsureStaticMeshRenderData() const;

      protected:
        std::filesystem::path MeshPath = GetDefaultQuadMeshPath();
        UStaticMesh* MeshAsset = nullptr;

        std::filesystem::path TexturePath;
        UTexture* TextureAsset = nullptr;
        bool      bBillboard = false;
        FVector   BillboardOffset = FVector(0.0f, 0.0f, 0.0f);

        mutable std::shared_ptr<FMeshData> MeshData;
        mutable std::shared_ptr<UMaterial> Material;
    };
} // namespace Engine::Component
