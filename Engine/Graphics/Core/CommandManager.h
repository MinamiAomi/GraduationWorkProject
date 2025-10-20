#pragma once
#include <mutex>
#include <vector>

#include <d3d12.h>

#include "CommandQueue.h"
#include "CommandAllocatorPool.h"
#include "CommandListPool.h"

// ListをまとめてQueueに投げる役割
class CommandManager {
public:
    CommandManager();

    void Create();
    // CommandListListを追加
    // Executeでまとめて投げられる
    // 次にExecuteを呼び出したときのフェンスの値が戻る
    UINT64 Add(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
    void Execute();

    CommandQueue& GetCommandQueue() { return commandQueue_; }
    CommandAllocatorPool& GetCommandAllocatorPool() { return commandAllocatorPool_; }
    CommandListPool& GetCommandListPool() { return commandListPool_; }

private:
    CommandManager(const CommandManager&) = delete;
    CommandManager& operator=(const CommandManager&) = delete;

    CommandQueue commandQueue_;
    CommandAllocatorPool commandAllocatorPool_;
    CommandListPool commandListPool_;
    std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> closedCommandLists_;
    std::mutex mutex_;
};
