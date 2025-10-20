#pragma once
#include "GPUResource.h"

#include <string>

#include "DescriptorHandle.h"

class GPUBuffer :
    public GPUResource {
public:

    void Create(const std::wstring& name, size_t numElements, size_t elementSize);

    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
    D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

    const DescriptorHandle& GetSRV() const { return srvHandle_; }
    const DescriptorHandle& GetUAV() const { return uavHandle_; }

    size_t GetBufferSize() const { return bufferSize_; }
    uint32_t GetNumElements() const { return numElements_; }
    uint32_t GetElementSize() const { return elementSize_; }

protected:
    GPUBuffer() : bufferSize_(0), numElements_(0), elementSize_(0),
        resourceFlags_(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
    }

    virtual void CreateViews() = 0;


    DescriptorHandle srvHandle_;
    DescriptorHandle uavHandle_;

    size_t bufferSize_ = 0;
    uint32_t numElements_ = 0;
    uint32_t elementSize_ = 0;
    D3D12_RESOURCE_FLAGS resourceFlags_;
};

class ByteAddressBuffer :
    public GPUBuffer {
public:
    virtual void CreateViews() override;
};

class StructuredBuffer :
    public GPUBuffer {
public:
    virtual void Destroy() override {
        GPUBuffer::Destroy();
    }
    
    virtual void CreateViews() override;
};


inline D3D12_VERTEX_BUFFER_VIEW GPUBuffer::GetVertexBufferView() const {
    D3D12_VERTEX_BUFFER_VIEW vbv{};
    vbv.BufferLocation = resource_->GetGPUVirtualAddress();
    vbv.SizeInBytes = UINT(bufferSize_);
    vbv.StrideInBytes = elementSize_;
    return vbv;
}

inline D3D12_INDEX_BUFFER_VIEW GPUBuffer::GetIndexBufferView() const {
    D3D12_INDEX_BUFFER_VIEW ibv{};
    ibv.BufferLocation = resource_->GetGPUVirtualAddress();
    ibv.SizeInBytes = UINT(bufferSize_);
    ibv.Format = elementSize_ == sizeof(UINT) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
    return ibv;
}
