#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <vector>
#include <queue>
#include <mutex>

class CommandAllocatorPool {
public:
    using CommandAllocatorPtr = Microsoft::WRL::ComPtr<ID3D12CommandAllocator>;

    explicit CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE typ);

    CommandAllocatorPtr Allocate(UINT64 completedFanceValue);
    void Discard(UINT64 fenceValue, const CommandAllocatorPtr& commandAllocator);

    size_t GetSize() const { return allocatorPool_.size(); }

private:
    const D3D12_COMMAND_LIST_TYPE type_;
    std::vector<CommandAllocatorPtr> allocatorPool_;
    std::queue<std::pair<UINT64, CommandAllocatorPtr>> readyAllocators_;
    std::mutex mutex_;
};