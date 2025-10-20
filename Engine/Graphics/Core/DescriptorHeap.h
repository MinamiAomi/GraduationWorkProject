#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>
#include <memory>
#include <mutex>

#include "FreeList.h"
#include "DescriptorHandle.h"

class DescriptorHeap :
    public std::enable_shared_from_this<DescriptorHeap> {
public:
    DescriptorHeap();

    void Create(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
    // Reset関数で解放できる
    // 任意に解放はできない
    DescriptorHandle Allocate();
    void Free(uint32_t index);

    operator ID3D12DescriptorHeap* () const { return descriptorHeap_.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
    D3D12_DESCRIPTOR_HEAP_TYPE type_;
    D3D12_CPU_DESCRIPTOR_HANDLE cpuStart_;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuStart_;
    uint32_t numDescriptors_;
    uint32_t descriptorSize_;
    FreeList freeList_;
    std::mutex mutex_;
};