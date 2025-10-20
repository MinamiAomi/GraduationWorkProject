#include "ShaderTable.h"

#include <cassert>

#include "../Core/Helper.h"

UINT ShaderRecord::GetLocalRootArgumentsSize(const D3D12_ROOT_PARAMETER* parameters, UINT numParameters) {
    UINT localRootArgumentsSize = 0;
    for (UINT i = 0; i < numParameters; ++i) {
        auto& parameter = parameters[i];
        switch (parameter.ParameterType) {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: {
            localRootArgumentsSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
            break;
        }
        case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS: {
            const UINT kuint32_tByteSize = sizeof(uint32_t);
            const UINT kConstantsAlignment = kuint32_tByteSize * 2;
            localRootArgumentsSize += Helper::AlignUp(kuint32_tByteSize * parameter.Constants.Num32BitValues, kConstantsAlignment);
            break;
        }
        case D3D12_ROOT_PARAMETER_TYPE_CBV:
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
        case D3D12_ROOT_PARAMETER_TYPE_UAV: {
            localRootArgumentsSize += sizeof(D3D12_GPU_VIRTUAL_ADDRESS);
            break;
        }
        default: {
            assert(false);
            break;
        }
        }
    }
    return localRootArgumentsSize;
}

ShaderRecord::ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize) :
    pShaderIdentifier_(pShaderIdentifier), shaderIdentifierSize_(shaderIdentifierSize) {
}

void ShaderRecord::Add(D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress) {
    auto& dest = localRootArguments_.emplace_back();
    memcpy(&dest, &gpuVirtualAddress, sizeof(gpuVirtualAddress));
}

void ShaderRecord::Add(D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle) {
    auto& dest = localRootArguments_.emplace_back();
    memcpy(&dest, &gpuDescriptorHandle, sizeof(gpuDescriptorHandle));
}

void ShaderRecord::Add(Helper::DWParam x, Helper::DWParam y) {
    struct DWParam2 {
        Helper::DWParam x;
        Helper::DWParam y;
    } dwParam2(x, y);
    assert(sizeof(dwParam2) == sizeof(UINT64));

    auto& dest = localRootArguments_.emplace_back();
    memcpy(&dest, &dwParam2, sizeof(dwParam2));
}



void ShaderRecord::CopyTo(void* dest) const {
    BYTE* byteDest = static_cast<BYTE*>(dest);
    memcpy(byteDest, pShaderIdentifier_, shaderIdentifierSize_);
    byteDest += shaderIdentifierSize_;
    if (!localRootArguments_.empty()) {
        memcpy(byteDest, localRootArguments_.data(), localRootArguments_.size() * sizeof(UINT64));
    }
}

void ShaderTable::Create(const std::wstring& name, const ShaderRecord* pShaderRecord, UINT numShaderRecords) {
    UINT maxShaderRecordSize = 0;
    for (UINT i = 0; i < numShaderRecords; ++i) {
        UINT size = pShaderRecord[i].GetSize();
        if (size > maxShaderRecordSize) {
            maxShaderRecordSize = size;
        }
    }

    shaderRecordSize_ = Helper::AlignUp(maxShaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
    numShaderRecords_ = numShaderRecords;
    UINT bufferSize = shaderRecordSize_ * numShaderRecords_;
    // 前回よりバッファサイズが増えた
    if (bufferSize > bufferSize_) {
        bufferSize_ = bufferSize;
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize_);
        CreateResource(name, heapProps, resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ);
        ASSERT_IF_FAILED(resource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedShaderRecords_)));
    }
    ZeroMemory(mappedShaderRecords_, bufferSize_);

    BYTE* currentMappedAddress = mappedShaderRecords_;
    for (UINT i = 0; i < numShaderRecords; ++i) {
        pShaderRecord[i].CopyTo(currentMappedAddress);
        currentMappedAddress += shaderRecordSize_;
    }
}
