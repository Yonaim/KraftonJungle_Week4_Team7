#pragma once

#include <memory>

#include "Asset/Cooked/StaticMeshCookedData.h"
#include "RHI/DynamicRHI.h"
#include "RHI/RHIBuffer.h"

namespace Asset
{
    struct FStaticMeshRenderResource
    {
        std::shared_ptr<RHI::FRHIVertexBuffer> VertexBuffer;
        std::shared_ptr<RHI::FRHIIndexBuffer>  IndexBuffer;

        uint32 VertexCount = 0;
        uint32 IndexCount = 0;

        static std::shared_ptr<FStaticMeshRenderResource>
        Create(const FStaticMeshCookedData& CookedData, RHI::FDynamicRHI& RHI);

        bool IsValid() const;
        void Reset();
    };
} // namespace Asset
