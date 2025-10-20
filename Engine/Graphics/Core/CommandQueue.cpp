#include "CommandQueue.h"

#include <cassert>
#include <sstream>

#include "Helper.h"
#include "Graphics.h"
#include "CommandContext.h"

CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type) :
    type_(type),
    fenceEvent_(nullptr),
    nextFenceValue_(0),
    lastCompletedFenceValue_(0) {
}

CommandQueue::~CommandQueue() {
    Destroy();
}

void CommandQueue::Create() {
    auto device = Graphics::GetInstance()->GetDevice();

    Destroy();

    D3D12_COMMAND_QUEUE_DESC desc{};
    desc.Type = type_;
    ASSERT_IF_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(commandQueue_.ReleaseAndGetAddressOf())));
    {
        // デバッグ用名前
        std::wostringstream name;
        name << L"CommandQueue ";
        name << Helper::GetCommandListTypeStr(type_);
        D3D12_OBJECT_SET_NAME(commandQueue_, name.str().c_str());
    }



    ASSERT_IF_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf())));
    {
        // デバッグ用名前
        std::wostringstream name;
        name << L"Fence ";
        name << Helper::GetCommandListTypeStr(type_);
        D3D12_OBJECT_SET_NAME(fence_, name.str().c_str());
    }


    if (fenceEvent_) {
        fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        assert(fenceEvent_);
    }

    lastCompletedFenceValue_ = 0;
    nextFenceValue_ = 1;
}

UINT64 CommandQueue::IncrementFence() {
    std::lock_guard<std::mutex> lock(fenceMutex_);
    commandQueue_->Signal(fence_.Get(), nextFenceValue_);
    return nextFenceValue_++;
}

bool CommandQueue::IsFenceComplete(UINT64 fenceValue) {
    if (fenceValue > lastCompletedFenceValue_) {
        lastCompletedFenceValue_ = std::max(lastCompletedFenceValue_, fence_->GetCompletedValue());
    }
    return fenceValue <= lastCompletedFenceValue_;
}

void CommandQueue::Wait(CommandQueue& commandQueue) {
    commandQueue_->Wait(commandQueue.fence_.Get(), commandQueue.nextFenceValue_ - 1);
}

void CommandQueue::WaitForGPU(UINT64 fenceValue) {
    if (IsFenceComplete(fenceValue)) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(eventMutex_);

        fence_->SetEventOnCompletion(fenceValue, fenceEvent_);
        WaitForSingleObject(fenceEvent_, INFINITE);
        lastCompletedFenceValue_ = fenceValue;
    }
}

void CommandQueue::Destroy() {
    if (commandQueue_) {
        WaitForIdle();
    }
    if (fenceEvent_) {
        CloseHandle(fenceEvent_);
        fenceEvent_ = nullptr;
    }
}

UINT64 CommandQueue::ExecuteCommandList(ID3D12CommandList* list) {
    std::lock_guard<std::mutex> lock(fenceMutex_);

    commandQueue_->ExecuteCommandLists(1, &list);

    commandQueue_->Signal(fence_.Get(), nextFenceValue_);

    return nextFenceValue_++;
}

UINT64 CommandQueue::ExecuteCommandLists(ID3D12CommandList** lists, UINT numLists) {
    std::lock_guard<std::mutex> lock(fenceMutex_);

    commandQueue_->ExecuteCommandLists(numLists, lists);
    commandQueue_->Signal(fence_.Get(), nextFenceValue_);

    return nextFenceValue_++;
}
