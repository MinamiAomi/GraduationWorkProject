#include "PixelBuffer.h"

#include "Externals/DirectXTex/Include/d3dx12.h"

#include <cassert>

#include "Helper.h"
#include "Graphics.h"

void PixelBuffer::CreateTextureResource(const std::wstring& name, const D3D12_RESOURCE_DESC& desc, D3D12_CLEAR_VALUE clearValue) {
    resource_.Reset();

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);    
    CreateResource(name, heapProps, desc, D3D12_RESOURCE_STATE_COMMON, &clearValue);
}

void PixelBuffer::AssociateWithResource(const std::wstring& name, ID3D12Resource* resource, D3D12_RESOURCE_STATES state) {
    assert(resource);
    auto desc = resource->GetDesc();

    resource_.Attach(resource);
    state_ = state;

    width_ = uint32_t(desc.Width);
    height_ = desc.Height;
    arraySize_ = desc.DepthOrArraySize;
    format_ = desc.Format;

    D3D12_OBJECT_SET_NAME(resource_, name.c_str());
}

D3D12_RESOURCE_DESC PixelBuffer::DescribeTex2D(
    uint32_t width, uint32_t height, uint32_t arraySize,
    DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags) {
    width_ = width;
    height_ = height;
    arraySize_ = arraySize;
    format_ = format;

    return CD3DX12_RESOURCE_DESC::Tex2D(format, UINT64(width), height, UINT16(arraySize), 1, 1, 0, flags);
}

