#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <unordered_map>
#include <vector>
#include <string>

struct RootSignatureDescHelper {

    struct DescriptorTableHelper {
        DescriptorTableHelper& AddCBVDescriptors(UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace = 0, UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
        DescriptorTableHelper& AddSRVDescriptors(UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace = 0, UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
        DescriptorTableHelper& AddUAVDescriptors(UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace = 0, UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
        DescriptorTableHelper& AddSampler(UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace = 0, UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

        std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
        bool hasSampler;
    };

    void AddConstantBufferView(UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    void AddShaderResourceView(UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    void AddUnorderedAccessView(UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    void AddConstants(UINT num32BitValues, UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    DescriptorTableHelper& AddDescriptorTable(D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

    void AddStaticSampler(UINT shaderRegister,
        D3D12_FILTER filter = D3D12_FILTER_ANISOTROPIC,
        D3D12_TEXTURE_ADDRESS_MODE addressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE addressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE addressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        FLOAT mipLODBias = 0,
        UINT maxAnisotropy = 16,
        D3D12_COMPARISON_FUNC comparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_STATIC_BORDER_COLOR borderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
        FLOAT minLOD = 0.f,
        FLOAT maxLOD = D3D12_FLOAT32_MAX,
        D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL,
        UINT registerSpace = 0);

    void SetFlag(D3D12_ROOT_SIGNATURE_FLAGS flag);
    void AddFlag(D3D12_ROOT_SIGNATURE_FLAGS flag);

    void Clear();

    operator D3D12_ROOT_SIGNATURE_DESC ();

    std::vector<D3D12_ROOT_PARAMETER> rootParameters;
    std::unordered_map<size_t, DescriptorTableHelper> descriptorTableHelperMaps_;
    std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
};

class RootSignature {
public:
    ~RootSignature() { Destroy(); }
    void Create(const std::wstring& name, const D3D12_ROOT_SIGNATURE_DESC& desc);

    operator ID3D12RootSignature* () const { return rootSignature_.Get(); }
    operator bool() const { return rootSignature_; }

    const Microsoft::WRL::ComPtr<ID3D12RootSignature>& Get() const { return rootSignature_; }

private:
    void Destroy();

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

#ifdef _DEBUG
    std::wstring name_;
#endif // _DEBUG
};