#include "RootSignature.h"

#include <cassert>

#include "Graphics.h"
#include "Helper.h"

RootSignatureDescHelper::DescriptorTableHelper& RootSignatureDescHelper::DescriptorTableHelper::AddCBVDescriptors(UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace, UINT offsetInDescriptorsFromTableStart) {
    assert(!hasSampler);
    CD3DX12_DESCRIPTOR_RANGE::Init(ranges.emplace_back(), D3D12_DESCRIPTOR_RANGE_TYPE_CBV, numDescriptors, baseShaderRegister, registerSpace, offsetInDescriptorsFromTableStart);
    return *this;
}

RootSignatureDescHelper::DescriptorTableHelper& RootSignatureDescHelper::DescriptorTableHelper::AddSRVDescriptors(UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace, UINT offsetInDescriptorsFromTableStart) {
    assert(!hasSampler);
    CD3DX12_DESCRIPTOR_RANGE::Init(ranges.emplace_back(), D3D12_DESCRIPTOR_RANGE_TYPE_SRV, numDescriptors, baseShaderRegister, registerSpace, offsetInDescriptorsFromTableStart);
    return *this;
}

RootSignatureDescHelper::DescriptorTableHelper& RootSignatureDescHelper::DescriptorTableHelper::AddUAVDescriptors(UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace, UINT offsetInDescriptorsFromTableStart) {
    assert(!hasSampler);
    CD3DX12_DESCRIPTOR_RANGE::Init(ranges.emplace_back(), D3D12_DESCRIPTOR_RANGE_TYPE_UAV, numDescriptors, baseShaderRegister, registerSpace, offsetInDescriptorsFromTableStart);
    return *this;
}

RootSignatureDescHelper::DescriptorTableHelper& RootSignatureDescHelper::DescriptorTableHelper::AddSampler(UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace, UINT offsetInDescriptorsFromTableStart) {
    assert(hasSampler || ranges.empty());
    CD3DX12_DESCRIPTOR_RANGE::Init(ranges.emplace_back(), D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, numDescriptors, baseShaderRegister, registerSpace, offsetInDescriptorsFromTableStart);
    return *this;
}

void RootSignatureDescHelper::AddConstantBufferView(UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility) {
    CD3DX12_ROOT_PARAMETER::InitAsConstantBufferView(rootParameters.emplace_back(), shaderRegister, registerSpace, visibility);
}

void RootSignatureDescHelper::AddShaderResourceView(UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility) {
    CD3DX12_ROOT_PARAMETER::InitAsShaderResourceView(rootParameters.emplace_back(), shaderRegister, registerSpace, visibility);
}

void RootSignatureDescHelper::AddUnorderedAccessView(UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility) {
    CD3DX12_ROOT_PARAMETER::InitAsUnorderedAccessView(rootParameters.emplace_back(), shaderRegister, registerSpace, visibility);
}

void RootSignatureDescHelper::AddConstants(UINT num32BitValues, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility) {
    CD3DX12_ROOT_PARAMETER::InitAsConstants(rootParameters.emplace_back(), num32BitValues, shaderRegister, registerSpace, visibility);
}

RootSignatureDescHelper::DescriptorTableHelper& RootSignatureDescHelper::AddDescriptorTable(D3D12_SHADER_VISIBILITY visibility) {
    auto& rootParameter = rootParameters.emplace_back();
    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter.ShaderVisibility = visibility;
    return descriptorTableHelperMaps_[rootParameters.size() - 1] = DescriptorTableHelper();
}

void RootSignatureDescHelper::AddStaticSampler(UINT shaderRegister, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressU, D3D12_TEXTURE_ADDRESS_MODE addressV, D3D12_TEXTURE_ADDRESS_MODE addressW, FLOAT mipLODBias, UINT maxAnisotropy, D3D12_COMPARISON_FUNC comparisonFunc, D3D12_STATIC_BORDER_COLOR borderColor, FLOAT minLOD, FLOAT maxLOD, D3D12_SHADER_VISIBILITY shaderVisibility, UINT registerSpace) {
    CD3DX12_STATIC_SAMPLER_DESC::Init(staticSamplers.emplace_back(), shaderRegister, filter, addressU, addressV, addressW, mipLODBias, maxAnisotropy, comparisonFunc, borderColor, minLOD, maxLOD, shaderVisibility, registerSpace);
}



void RootSignatureDescHelper::SetFlag(D3D12_ROOT_SIGNATURE_FLAGS flag) {
    flags = flag;
}

void RootSignatureDescHelper::AddFlag(D3D12_ROOT_SIGNATURE_FLAGS flag) {
    flags |= flag;
}

void RootSignatureDescHelper::Clear() {
    rootParameters.clear();
    descriptorTableHelperMaps_.clear();
    staticSamplers.clear();
    flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
}

RootSignatureDescHelper::operator D3D12_ROOT_SIGNATURE_DESC() {
    for (size_t i = 0; i < rootParameters.size(); ++i) {
        if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
            assert(!descriptorTableHelperMaps_[i].ranges.empty());
            rootParameters[i].DescriptorTable.NumDescriptorRanges = (UINT)descriptorTableHelperMaps_[i].ranges.size();
            rootParameters[i].DescriptorTable.pDescriptorRanges = descriptorTableHelperMaps_[i].ranges.data();
        }
    }

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.Flags = flags;
    rootSignatureDesc.pParameters = rootParameters.data();
    rootSignatureDesc.NumParameters = (UINT)rootParameters.size();
    rootSignatureDesc.pStaticSamplers = staticSamplers.data();
    rootSignatureDesc.NumStaticSamplers = (UINT)staticSamplers.size();
    return rootSignatureDesc;
}

void RootSignature::Create(const std::wstring& name, const D3D12_ROOT_SIGNATURE_DESC& desc) {
    Destroy();

    Microsoft::WRL::ComPtr<ID3DBlob> blob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    /* auto d = desc;
     d.Flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;*/

    if (FAILED(D3D12SerializeRootSignature(
        &desc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        blob.GetAddressOf(),
        errorBlob.GetAddressOf()))) {

        MessageBoxA(nullptr, static_cast<char*>(errorBlob->GetBufferPointer()), "Failed create RootSignature!!", S_OK);
        OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    ASSERT_IF_FAILED(Graphics::GetInstance()->GetDevice()->CreateRootSignature(
        0,
        blob->GetBufferPointer(),
        blob->GetBufferSize(),
        IID_PPV_ARGS(rootSignature_.ReleaseAndGetAddressOf())));
    D3D12_OBJECT_SET_NAME(rootSignature_, name.c_str());
#ifdef _DEBUG
    name_ = name;
#endif // _DEBUG
}

void RootSignature::Destroy() {
    if (rootSignature_) {
        Graphics::GetInstance()->GetReleasedObjectTracker().AddObject(rootSignature_);
        rootSignature_ = nullptr;
    }
}


