#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>
#include <memory>

#include "RootSignature.h"
#include "DescriptorHandle.h"
#include "DescriptorHeap.h"
#include "CommandManager.h"
#include "ReleasedObjectTracker.h"
#include "LinearAllocator.h"

#define BINDLESS_RESOURCE_MAX (1 << 13)
#define DXR_DEVICE ID3D12Device5

class Graphics {
public:
    static Graphics* GetInstance();

    void Initialize();
    void Finalize();

    DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);

    ID3D12Device* GetDevice() const { return device_.Get(); }
    DXR_DEVICE* GetDXRDevoce() const { return dxrDevice_.Get(); }
    CommandManager& GetCommandManager() { return commandManager_; }
    DescriptorHeap& GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) { return *descriptorHeaps_[type]; }
    LinearAllocatorPagePool& GetLinearAllocatorPagePool(LinearAllocatorType type) { return linearAllocatorPagePools_[type]; }
    ReleasedObjectTracker& GetReleasedObjectTracker() { return releasedObjectTracker_; }

    RootSignature& GetDynamicResourcesRootSignature() { return dynamicResourcesRootSignature_; }

    bool IsDXRSupported() const { return dxrDevice_; }

    void CheckDRED(HRESULT presentReturnValue);

private:
    static const uint32_t kNumRTVs = 64;
    static const uint32_t kNumDSVs = 2;
    static const uint32_t kNumSRVs = BINDLESS_RESOURCE_MAX;
    static const uint32_t kNumSamplers = 16;

    Graphics();
    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;
    ~Graphics() = default;

    void CreateDevice();
    void CheckFeatureSupport();
    void CreateDynamicResourcesRootSignature();

    ReleasedObjectTracker releasedObjectTracker_;

    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    Microsoft::WRL::ComPtr<DXR_DEVICE> dxrDevice_;

    // directのみ
    CommandManager commandManager_;

    std::shared_ptr<DescriptorHeap> descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    LinearAllocatorPagePool linearAllocatorPagePools_[LinearAllocatorType::Count];
    
    RootSignature dynamicResourcesRootSignature_;
};
