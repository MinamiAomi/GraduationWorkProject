#include "DescriptorHeap.h"

#include <cassert>
#include <sstream>

#include "Graphics.h"
#include "Helper.h"

DescriptorHeap::DescriptorHeap() :
    type_(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES),
    cpuStart_(D3D12_CPU_DESCRIPTOR_HANDLE_NULL),
    gpuStart_(D3D12_GPU_DESCRIPTOR_HANDLE_NULL),
    numDescriptors_(0),
    descriptorSize_(0) {
}

void DescriptorHeap::Create(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) {
    assert(type != D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);

    auto device = Graphics::GetInstance()->GetDevice();

    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Type = type;
    desc.NumDescriptors = numDescriptors;
    if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    }
    ASSERT_IF_FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(descriptorHeap_.ReleaseAndGetAddressOf())));
    {
        // デバッグ用名前
        std::wostringstream name;
        name << L"DescriptorHeap ";
        name << Helper::GetDescriptorHeapTypeStr(type);
        D3D12_OBJECT_SET_NAME(descriptorHeap_, name.str().c_str());
    }

    type_ = type;
    cpuStart_ = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    if (desc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
        gpuStart_ = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    }
    descriptorSize_ = device->GetDescriptorHandleIncrementSize(type_);
    numDescriptors_ = numDescriptors;

    freeList_.Resize(numDescriptors_);
}

DescriptorHandle DescriptorHeap::Allocate() {
    std::lock_guard<std::mutex> lock(mutex_);
    // もう空きがないので拡張しましょう
    assert(freeList_.FreeCount() != 0);
    // フリーリスト
    uint32_t allocationIndex = freeList_.Allocate();

    DescriptorHandle allocationHandle;
    allocationHandle.index_ = allocationIndex;
    allocationHandle.heap_ = shared_from_this();
    // CPUハンドルを計算
    allocationHandle.cpu_ = cpuStart_;
    allocationHandle.cpu_.ptr += allocationHandle.index_ * SIZE_T(descriptorSize_);
    // GPUハンドルを計算
    if (gpuStart_.ptr != 0) {
        allocationHandle.gpu_ = gpuStart_;
        allocationHandle.gpu_.ptr += allocationHandle.index_ * UINT64(descriptorSize_);
    }
    return allocationHandle;
}

void DescriptorHeap::Free(uint32_t index) {
    std::lock_guard<std::mutex> lock(mutex_);
    freeList_.Free(index);
}
