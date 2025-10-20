#include "UploadBuffer.h"

#include <cassert>

#include "Helper.h"
#include "Graphics.h"

UploadBuffer::~UploadBuffer() {
    Destroy();
}

void UploadBuffer::Create(const std::wstring& name, size_t bufferSize) {
    Destroy();

    auto desc = CD3DX12_RESOURCE_DESC::Buffer(UINT64(bufferSize));
    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    CreateResource(name, heapProps, desc, D3D12_RESOURCE_STATE_GENERIC_READ);

    bufferSize_ = bufferSize;
    ASSERT_IF_FAILED(resource_->Map(0, nullptr, &cpuDataBegin_));
}

void UploadBuffer::Create(const std::wstring& name, size_t numElements, size_t elementSize) {
    Create(name, numElements * elementSize);
}

void UploadBuffer::CreateConstantBuffer(const std::wstring& name, size_t bufferSize) {
    Create(name, Helper::AlignUp(bufferSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));
}

void UploadBuffer::Copy(const void* srcData, size_t copySize) const {
    assert(copySize <= bufferSize_);
    memcpy(cpuDataBegin_, srcData, copySize);
}

