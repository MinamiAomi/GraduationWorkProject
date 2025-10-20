#pragma once
#include "GPUResource.h"

#include <memory>
#include <queue>
#include <mutex>

struct LinearAllocatorType {
    enum Type {
        Default,
        Upload,

        Count
    } type;
    LinearAllocatorType(Type type = Default) : type(type) {}
    operator Type() { return type; }
    const size_t GetSize() const {
        switch (type) {
        case LinearAllocatorType::Default: return 0x500000; // 64KiB
        case LinearAllocatorType::Upload: return 0x200000; // 2MiB
        }
        return (size_t)-1;
    }
    const wchar_t* GetName() {
        switch (type) {
        case LinearAllocatorType::Default: return  L"Default";
        case LinearAllocatorType::Upload: return L"Upload";
        }
        return L"";
    }
};

class LinearAllocatorPage :
    public GPUResource {
public:
    void Create(const std::wstring& name, LinearAllocatorType type);

    void* GetCPUAddressStart() const { return cpuAddressStart_; }
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddressStart() const { return gpuAddressStart_; }
    size_t GetSize() const { return size_; }

private:
    void* cpuAddressStart_;
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddressStart_;
    size_t size_;
};

class LinearAllocatorPagePool {
public:
    void Initialize(LinearAllocatorType type);
    void Finalize();

    LinearAllocatorPage* Allocate();
    void Discard(D3D12_COMMAND_LIST_TYPE commandType, UINT64 fenceValue, const std::vector<LinearAllocatorPage*>& pages);
    void Clear();

    size_t GetSize() const { return pagePool_.size(); }

private:
    using ReadyPageQueue = std::queue<std::pair<UINT64, LinearAllocatorPage*>>;

    ReadyPageQueue& GetReadyPageQueue(D3D12_COMMAND_LIST_TYPE commandType) {
        switch (commandType) {
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            return computeReadyPageQueue_;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            return copyReadyPageQueue_;
        }
        return directReadyPageQueue_;
    }
    bool TryAllocate(D3D12_COMMAND_LIST_TYPE commandType, LinearAllocatorPage** page);
    LinearAllocatorPage* CreateNewPage();

    LinearAllocatorType type_;
    std::vector<std::unique_ptr<LinearAllocatorPage>> pagePool_;
    ReadyPageQueue directReadyPageQueue_;
    ReadyPageQueue computeReadyPageQueue_;
    ReadyPageQueue copyReadyPageQueue_;
    std::mutex mutex_;
};

class LinearAllocator {
public:
    struct Allocation {
        GPUResource& resource;
        void* cpu;
        const D3D12_GPU_VIRTUAL_ADDRESS gpu;
        size_t size;
        size_t offset;
    };

    void Create(LinearAllocatorType type);
    Allocation Allocate(size_t size, size_t alignment = 256);
    void Reset(D3D12_COMMAND_LIST_TYPE commandType, UINT64 fenceValue);

private:
    bool HasSpace(size_t size, size_t alignment);

    LinearAllocatorType type_;

    std::vector<LinearAllocatorPage*> usedPages_;
    LinearAllocatorPage* currentPage_ = nullptr;
    size_t currentOffset_ = 0;
    std::mutex mutex_;
};
