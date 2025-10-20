#include "SamplerManager.h"

#include "Graphics.h"

namespace SamplerManager {
    
    DescriptorHandle AnisotropicWrap;
    DescriptorHandle LinearWrap;
    DescriptorHandle LinearClamp;
    DescriptorHandle LinearBorder;
    DescriptorHandle LinearMirror;

    DescriptorHandle PointWrap;
    DescriptorHandle PointClamp;
    DescriptorHandle PointBorder;
    DescriptorHandle PointMirror;

    void Initialize() {

        auto graphics = Graphics::GetInstance();
        auto device = graphics->GetDevice();

        D3D12_SAMPLER_DESC desc{};
        desc.Filter = D3D12_FILTER_COMPARISON_ANISOTROPIC;
        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
        desc.MaxAnisotropy = D3D12_DEFAULT_MAX_ANISOTROPY;
        desc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        float borderColor[4] = { 0.0f,0.0f,0.0f,0.0f };
        memcpy(desc.BorderColor, borderColor, sizeof(desc.BorderColor));
        desc.MinLOD = 0.0f;
        desc.MaxLOD = D3D12_FLOAT32_MAX;

        AnisotropicWrap = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&desc, AnisotropicWrap);

        desc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        LinearWrap = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&desc, LinearWrap);

        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

        LinearClamp = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&desc, LinearClamp);

        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

        LinearBorder = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&desc, LinearBorder);

        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        LinearMirror = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&desc, LinearMirror);

        desc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

        PointWrap = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&desc, PointWrap);

        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

        PointClamp = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&desc, PointClamp);
    
        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

        PointBorder = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&desc, PointBorder);
    
        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        PointMirror = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&desc, PointMirror);
    }

}