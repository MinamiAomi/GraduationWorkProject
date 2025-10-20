#include "CommandContext.h"

#include <vector>
#ifdef _DEBUG
//#include <pix3.h>
#endif // _DEBUG


#include "Helper.h"
#include "Graphics.h"
#include "DescriptorHeap.h"


void CommandContext::Start(D3D12_COMMAND_LIST_TYPE type) {
    // 残っているはずがない
    assert(commandAllocator_ == nullptr);

    type_ = type;

    auto graphics = Graphics::GetInstance();
    device_ = graphics->GetDevice();
    auto& commandManager = graphics->GetCommandManager();
    commandAllocator_ = commandManager.GetCommandAllocatorPool().Allocate(commandManager.GetCommandQueue().GetLastCompletedFenceValue());
    commandList_ = commandManager.GetCommandListPool().Allocate(commandAllocator_);
    
    for (int i = 0; i < LinearAllocatorType::Count; ++i) {
        dynamicBuffers_[i].Create(LinearAllocatorType::Type(i));
    }

    if (graphics->IsDXRSupported()) {
        ASSERT_IF_FAILED(commandList_.As(&dxrCommandList_));
    }


    resourceHeap_ = (ID3D12DescriptorHeap*)graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    samplerHeap_ = (ID3D12DescriptorHeap*)graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    ID3D12DescriptorHeap* ppHeaps[] = {
        resourceHeap_,
        samplerHeap_
    };
    commandList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    rootSignature_ = nullptr;
    pipelineState_ = nullptr;
    primitiveTopology_ = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

    isClose_ = false;
}

void CommandContext::Close() {
    assert(!isClose_);
    // バリアをフラッシュ
    FlushResourceBarriers();
    ASSERT_IF_FAILED(commandList_->Close());
    isClose_ = true;
}

UINT64 CommandContext::Finish(bool waitForCompletion) {
    waitForCompletion;
    if (!isClose_) {
        Close();
    }

    auto graphics = Graphics::GetInstance();
    auto& commandManager = graphics->GetCommandManager();


    UINT64 fenceValue = commandManager.Add(commandList_);
    
    commandManager.GetCommandAllocatorPool().Discard(fenceValue, commandAllocator_);
    commandAllocator_ = nullptr;
    
    // コマンドマネージャーにDiscardを以降した
    //commandManager.GetCommandListPool().Discard(commandList_);
    //commandList_ = nullptr;
    
    dxrCommandList_ = nullptr;
    for (int i = 0; i < LinearAllocatorType::Count; ++i) {
        dynamicBuffers_[i].Reset(type_, fenceValue);
    }

    return fenceValue;
}

void CommandContext::SetMarker(const std::wstring& label) {
    label;
#ifdef _DEBUG
    //PIXSetMarker(commandList_.Get(), 0, label.c_str());
    //commandList_->SetMarker(0, label.data(), UINT(label.size() * sizeof(label[0])));
#endif // _DEBUG
}


void CommandContext::BeginEvent(const std::wstring& label) {
    label;
#ifdef _DEBUG
    //PIXBeginEvent(commandList_.Get(), 0, label.c_str());
    //commandList_->BeginEvent(0, label.data(), UINT(label.size() * sizeof(label[0])));
#endif // _DEBUG
}

void CommandContext::EndEvent() {
#ifdef _DEBUG
    //PIXEndEvent(commandList_.Get());
    //commandList_->EndEvent();
#endif // _DEBUG
}