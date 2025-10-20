#include "CommandAllocatorPool.h"

#include <sstream>

#include "Helper.h"
#include "Graphics.h"

CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type) :
    type_(type) {
}

CommandAllocatorPool::CommandAllocatorPtr CommandAllocatorPool::Allocate(UINT64 completedFanceValue) {
    std::lock_guard<std::mutex> lock(mutex_);

    CommandAllocatorPtr commandAllocator = nullptr;

    if (!readyAllocators_.empty()) {
        const auto& [readyFenceValue, readyCommandAllocator] = readyAllocators_.front();
        if (readyFenceValue <= completedFanceValue) {
            commandAllocator = readyCommandAllocator;
            ASSERT_IF_FAILED(commandAllocator->Reset());
            readyAllocators_.pop();
        }
    }

    if (!commandAllocator) {
        auto device = Graphics::GetInstance()->GetDevice();
        ASSERT_IF_FAILED(device->CreateCommandAllocator(type_, IID_PPV_ARGS(commandAllocator.GetAddressOf())));
        
        allocatorPool_.emplace_back(commandAllocator);

        // デバッグ用名前
        std::wostringstream name;
        name << L"CommandAllocator ";
        name << Helper::GetCommandListTypeStr(type_);
        name << L" ";
        name << allocatorPool_.size();
        D3D12_OBJECT_SET_NAME(commandAllocator, name.str().c_str());
    }

    return commandAllocator;
}

void CommandAllocatorPool::Discard(UINT64 fenceValue, const CommandAllocatorPtr& commandAllocator) {
    std::lock_guard<std::mutex> lock(mutex_);

    readyAllocators_.push(std::make_pair(fenceValue, commandAllocator));
}

