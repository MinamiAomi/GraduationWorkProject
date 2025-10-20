#include "CommandListPool.h"

#include <sstream>

#include "Graphics.h"
#include "Helper.h"

CommandListPool::CommandListPool(D3D12_COMMAND_LIST_TYPE type) :
    type_(type) {
}

CommandListPool::GraphicsCommandListPtr CommandListPool::Allocate(const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator) {
    std::lock_guard<std::mutex> lock(mutex_);

    GraphicsCommandListPtr commandList = nullptr;
    
    if (!readyCommandLists_.empty()) {
        commandList = readyCommandLists_.front();
        commandList->Reset(commandAllocator.Get(), nullptr);
        readyCommandLists_.pop();
    }

    if (!commandList) {
        auto device = Graphics::GetInstance()->GetDevice();
        ASSERT_IF_FAILED(device->CreateCommandList(1, type_, commandAllocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

        commandListPool_.emplace_back(commandList);

        // デバッグ用名前
        std::wostringstream name;
        name << L"CommandList ";
        name << Helper::GetCommandListTypeStr(type_);
        name << L" ";
        name << commandListPool_.size();
        D3D12_OBJECT_SET_NAME(commandList, name.str().c_str());
    }

    return commandList;
}

void CommandListPool::Discard(const GraphicsCommandListPtr& list) {
    std::lock_guard<std::mutex> lock(mutex_);

    readyCommandLists_.push(list);
}
