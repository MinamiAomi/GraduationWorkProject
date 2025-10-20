#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>
#include <mutex>

#include "CommandAllocatorPool.h"
#include "CommandListPool.h"

class CommandQueue {
    friend class CommandContext;
public:
    CommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
    ~CommandQueue();

    void Create();
    
    UINT64 IncrementFence();
    bool IsFenceComplete(UINT64 fenceValue);
    void Wait(CommandQueue& commandQueue);
    void WaitForGPU(UINT64 fenceValue);
    void WaitForIdle() { WaitForGPU(IncrementFence()); }

    UINT64 ExecuteCommandList(ID3D12CommandList* list);
    UINT64 ExecuteCommandLists(ID3D12CommandList** lists, UINT numLists);

    operator ID3D12CommandQueue* () const { return commandQueue_.Get(); }
    
    UINT64 GetNextFenceValue() const { return nextFenceValue_; }
    UINT64 GetLastCompletedFenceValue() const { return lastCompletedFenceValue_; }

private:
    void Destroy();
    //UINT64 ExecuteCommandList(ID3D12CommandList* list);

    const D3D12_COMMAND_LIST_TYPE type_;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    HANDLE fenceEvent_;
    UINT64 nextFenceValue_;
    UINT64 lastCompletedFenceValue_;
    std::mutex fenceMutex_;
    std::mutex eventMutex_;
};
