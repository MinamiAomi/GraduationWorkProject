#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <vector>
#include <queue>
#include <mutex>

class CommandListPool {
public:
    using GraphicsCommandListPtr = Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>;

    CommandListPool(D3D12_COMMAND_LIST_TYPE type);

    GraphicsCommandListPtr Allocate(const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator);
    void Discard(const GraphicsCommandListPtr& list);

    size_t GetSize() const { return commandListPool_.size(); }

private:
    const D3D12_COMMAND_LIST_TYPE type_;
    std::vector<GraphicsCommandListPtr> commandListPool_;
    std::queue<GraphicsCommandListPtr> readyCommandLists_;
    std::mutex mutex_;
};