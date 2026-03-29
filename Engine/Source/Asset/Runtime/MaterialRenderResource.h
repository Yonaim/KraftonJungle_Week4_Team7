#pragma once

#include <memory>

#include "Asset/Cooked/MaterialCookedData.h"
#include "RHI/DynamicRHI.h"
#include "RHI/RHIBuffer.h"
#include "RHI/RHITexture.h"

namespace Asset
{
    struct FMaterialRenderResource
    {
        std::shared_ptr<RHI::FRHITexture>        BaseColorTexture;
        std::shared_ptr<RHI::FRHITexture>        NormalTexture;
        std::shared_ptr<RHI::FRHITexture>        ORMTexture;
        std::shared_ptr<RHI::FRHIConstantBuffer> ParameterBuffer;

        static std::shared_ptr<FMaterialRenderResource>
        Create(const FMaterialCookedData& CookedData, RHI::FDynamicRHI& RHI);

        bool IsValid() const;
        void Reset();

      private:
        struct FMaterialParameters
        {
            FVector DiffuseColor = FVector(1.0f, 1.0f, 1.0f);
            float   Opacity = 1.0f;
            FVector AmbientColor = FVector(0.0f, 0.0f, 0.0f);
            float   Shininess = 0.0f;
            FVector SpecularColor = FVector(0.0f, 0.0f, 0.0f);
            float   Padding = 0.0f;
        };

        static std::shared_ptr<RHI::FRHITexture>
        CreateTextureForSlot(const FMaterialCookedData& CookedData, EMaterialTextureSlot Slot,
                             RHI::FDynamicRHI& RHI);

        static std::shared_ptr<RHI::FRHIConstantBuffer>
        CreateParameterBuffer(const FMaterialCookedData& CookedData, RHI::FDynamicRHI& RHI);
    };
} // namespace Asset
