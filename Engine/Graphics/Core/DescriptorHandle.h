#pragma once

#include <d3d12.h>

#include <cstdint>
#include <memory>

#define D3D12_CPU_DESCRIPTOR_HANDLE_NULL (D3D12_CPU_DESCRIPTOR_HANDLE(0))
#define D3D12_GPU_DESCRIPTOR_HANDLE_NULL (D3D12_GPU_DESCRIPTOR_HANDLE(0))

class DescriptorHeap;

class DescriptorHandle {
    friend class DescriptorHeap;
public:
    DescriptorHandle();
    DescriptorHandle(const DescriptorHandle&) = delete;
    DescriptorHandle& operator=(const DescriptorHandle&) = delete;
    DescriptorHandle(DescriptorHandle&& move);
    DescriptorHandle& operator=(DescriptorHandle&& move);
    ~DescriptorHandle();

    operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return cpu_; }
    operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return gpu_; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPU() const { return cpu_; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPU() const { return gpu_; }
    bool IsShaderVisible() const { return gpu_.ptr != 0; }
    bool IsNull() const { return cpu_.ptr == 0; }
    uint32_t GetIndex() const { return index_; }

protected:
    void Free();

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_;
    D3D12_GPU_DESCRIPTOR_HANDLE gpu_;
    uint32_t index_;
    std::weak_ptr<DescriptorHeap> heap_;
};