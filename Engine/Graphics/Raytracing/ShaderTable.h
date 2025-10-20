#pragma once
#include "../Core/GPUResource.h"
#include "../Core/Helper.h"

#include <vector>

class ShaderRecord {
public:
    static UINT GetLocalRootArgumentsSize(const D3D12_ROOT_PARAMETER* parameters, UINT numParameters);

    ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    void Add(D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress);
    void Add(D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle);
    void Add(Helper::DWParam x, Helper::DWParam y);
    
    void CopyTo(void* dest) const;

    UINT GetSize() const { return (UINT)(shaderIdentifierSize_ + localRootArguments_.size() * sizeof(UINT64)); }

private:
    void* pShaderIdentifier_;
    UINT shaderIdentifierSize_;
    std::vector<UINT64> localRootArguments_;
};

class ShaderTable :
    public GPUResource {
public:
    void Create(const std::wstring& name, const ShaderRecord* pShaderRecord, UINT numShaderRecords);
    UINT GetShaderRecordSize()const { return shaderRecordSize_; }
    UINT GetBufferSize() const { return bufferSize_; }

private:
    UINT bufferSize_;
    UINT shaderRecordSize_;
    UINT numShaderRecords_;
    BYTE* mappedShaderRecords_;
};