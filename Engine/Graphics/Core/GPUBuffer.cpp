#include "GPUBuffer.h"

#include <cassert>

#include "Helper.h"
#include "Graphics.h"

void GPUBuffer::Create(const std::wstring& name, size_t numElements, size_t elementSize) {
    Destroy();

    numElements_ = uint32_t(numElements);
    elementSize_ = uint32_t(elementSize);
    bufferSize_ = numElements * elementSize;

    auto desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize_, resourceFlags_);
    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    CreateResource(name, heapProps, desc);

    CreateViews();
}

void ByteAddressBuffer::CreateViews() {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
    srvDesc.Buffer.NumElements = UINT(bufferSize_ / 4);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    uavDesc.Buffer.NumElements = UINT(bufferSize_ / 4);

    auto graphics = Graphics::GetInstance();
    if (srvHandle_.IsNull()) {
        srvHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
    if (uavHandle_.IsNull()) {
        uavHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    auto device = graphics->GetDevice();
    device->CreateShaderResourceView(resource_.Get(), &srvDesc, srvHandle_);
    device->CreateUnorderedAccessView(resource_.Get(), nullptr, &uavDesc, uavHandle_);
}

void StructuredBuffer::CreateViews() {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = UINT(numElements_);
    srvDesc.Buffer.StructureByteStride = UINT(elementSize_);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.NumElements = UINT(numElements_);
    uavDesc.Buffer.StructureByteStride = UINT(elementSize_);

    auto graphics = Graphics::GetInstance();
    if (srvHandle_.IsNull()) {
        srvHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
    if (uavHandle_.IsNull()) {
        uavHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    auto device = graphics->GetDevice();
    device->CreateShaderResourceView(resource_.Get(), &srvDesc, srvHandle_);
    device->CreateUnorderedAccessView(resource_.Get(), nullptr, &uavDesc, uavHandle_);
}