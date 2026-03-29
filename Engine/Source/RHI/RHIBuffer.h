#pragma once

#include "RHI/RHIResource.h"
#include "RHI/RHITypes.h"

namespace RHI
{

    // ======================== Buffer Resource ==========================

    class FRHIBuffer : public FRHIResource
    {
      public:
        virtual ~FRHIBuffer() = default;

        const FBufferDesc& GetDesc() const { return Desc; }

      protected:
        FBufferDesc Desc;
    };

    // ======================== Vertex Buffer ==========================

    class FRHIVertexBuffer : public FRHIBuffer
    {
      public:
        virtual ~FRHIVertexBuffer() = default;
    };

    // ======================== Index Buffer ==========================

    class FRHIIndexBuffer : public FRHIBuffer
    {
      public:
        virtual ~FRHIIndexBuffer() = default;

        EIndexFormat GetIndexFormat() const { return IndexFormat; }

      protected:
        EIndexFormat IndexFormat = EIndexFormat::UInt32;
    };

    // ======================== Constant Buffer ==========================

    class FRHIConstantBuffer : public FRHIBuffer
    {
      public:
        virtual ~FRHIConstantBuffer() = default;
    };

} // namespace RHI
