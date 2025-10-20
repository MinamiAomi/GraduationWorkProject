#include "LinearAllocator.h"

#include <assert.h>
#include <sstream>

#include "Graphics.h"
#include "Helper.h"

//
//void LinearAllocator::Finalize() {
//    pagePool_.Clear();
//}
//
//LinearAllocator::Allocation LinearAllocator::Allocate(size_t sizeInByte, size_t alignment) {
//    assert(sizeInByte <= kPageSize);
//
//     現在のページがない、または、ページに空きがない場合
//    if (!currentPage_ || !currentPage_->HasSpace(sizeInByte, alignment)) {
//        if (currentPage_) {
//            usedPages_.emplace_back(currentPage_);
//        }
//         新しいページを要求
//        currentPage_ = pagePool_.Allocate();
//    }
//     割り当て
//    return currentPage_->Allocate(sizeInByte, alignment);
//}
//
//void LinearAllocator::Reset(D3D12_COMMAND_LIST_TYPE type, UINT64 fenceValue) {
//    if (currentPage_) {
//        usedPages_.emplace_back(currentPage_);
//    }
//    if (!usedPages_.empty()) {
//        pagePool_.Discard(type, fenceValue, usedPages_);
//        usedPages_.clear();
//    }
//}
//
//LinearAllocator::Page::Page() :
//    offset_(0) {
//    buffer_.Create(L"LinearAllocator Page", LinearAllocator::kPageSize);
//}
//
//bool LinearAllocator::Page::HasSpace(size_t sizeInByte, size_t alignment) {
//    size_t alignedSize = Helper::AlignUp(sizeInByte, alignment);
//    size_t alignedOffset = Helper::AlignUp(offset_, alignment);
//
//    return alignedOffset + alignedSize <= buffer_.GetBufferSize();
//}
//
//LinearAllocator::Allocation LinearAllocator::Page::Allocate(size_t sizeInByte, size_t alignment) {
//    size_t alignedSize = Helper::AlignUp(sizeInByte, alignment);
//    offset_ = Helper::AlignUp(offset_, alignment);
//
//    Allocation allocation{
//        .cpu = static_cast<uint8_t*>(buffer_.GetCPUDataBegin()) + offset_,
//        .gpu = buffer_.GetGPUVirtualAddress() + offset_
//    };
//    offset_ += alignedSize;
//    return allocation;
//}
//
//void LinearAllocator::Page::Reset() {
//    offset_ = 0;
//}
//
//LinearAllocator::PagePtr LinearAllocator::PagePool::Allocate() {
//    std::lock_guard<std::mutex> lock(mutex_);
//
//    PagePtr page;
//
//    page = TryAllocate(D3D12_COMMAND_LIST_TYPE_DIRECT);
//    if (page) { return page; }
//    page = TryAllocate(D3D12_COMMAND_LIST_TYPE_COMPUTE);
//    if (page) { return page; }
//    page = TryAllocate(D3D12_COMMAND_LIST_TYPE_COPY);
//    if (page) { return page; }
//
//    page = std::make_shared<Page>();
//    pagePool_.emplace_back(page);
//
//    return page;
//}
//
//void LinearAllocator::PagePool::Discard(D3D12_COMMAND_LIST_TYPE type, UINT64 fenceValue, const std::vector<PagePtr>& pages) {
//    auto& readyPages = GetReadyPages(type);
//    for (auto& page : pages) {
//        readyPages.push(std::make_pair(fenceValue, page));
//    }
//}
//
//void LinearAllocator::PagePool::Clear() {
//    pagePool_.clear();
//    std::queue<std::pair<UINT64, PagePtr>>().swap(directReadyPages_);
//    std::queue<std::pair<UINT64, PagePtr>>().swap(computeReadyPages_);
//    std::queue<std::pair<UINT64, PagePtr>>().swap(copyReadyPages_);
//}
//
//LinearAllocator::PagePtr LinearAllocator::PagePool::TryAllocate(D3D12_COMMAND_LIST_TYPE type) {
//    auto& readyPages = GetReadyPages(type);
//
//    if (!readyPages.empty()) {
//        const auto& [fenceValue, readyPage] = readyPages.front();
//        auto& queue = Graphics::GetInstance()->GetCommandQueue(type);
//
//        if (fenceValue <= queue.GetLastCompletedFenceValue()) {
//            readyPage->Reset();
//            readyPages.pop();
//            return readyPage;
//        }
//    }
//
//    return PagePtr();
//}

//LinearAllocatorPage::LinearAllocatorPage(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES state) {
//    assert(resource);
//    resource_ = resource;
//    state_ = state;
//    gpuAddressStart_ = resource_->GetGPUVirtualAddress();
//    D3D12_HEAP_PROPERTIES heapProps{};
//    D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;
//    ASSERT_IF_FAILED(resource_->GetHeapProperties(&heapProps, &heapFlags));
//    D3D12_RESOURCE_DESC desc = resource_->GetDesc();
//    size_ = (size_t)desc.Width;
//    if (heapProps.Type != D3D12_HEAP_TYPE_DEFAULT) {
//        ASSERT_IF_FAILED(resource_->Map(0, nullptr, &cpuAddressStart_));
//    }
//}

void LinearAllocatorPagePool::Initialize(LinearAllocatorType type) {
    type_ = type;
}

void LinearAllocatorPagePool::Finalize() {
    Clear();
}

LinearAllocatorPage* LinearAllocatorPagePool::Allocate() {
    std::lock_guard lock(mutex_);

    LinearAllocatorPage* page = nullptr;

    if (TryAllocate(D3D12_COMMAND_LIST_TYPE_DIRECT, &page)) { return page; }
    if (TryAllocate(D3D12_COMMAND_LIST_TYPE_COMPUTE, &page)) { return page; }
    if (TryAllocate(D3D12_COMMAND_LIST_TYPE_COPY, &page)) { return page; }

    page = CreateNewPage();
    return page;
}

void LinearAllocatorPagePool::Discard(D3D12_COMMAND_LIST_TYPE commandType, UINT64 fenceValue, const std::vector<LinearAllocatorPage*>& pages) {
    std::lock_guard lock(mutex_);
    auto& readyPageQueue = GetReadyPageQueue(commandType);
    for (auto& page : pages) {
        readyPageQueue.push(std::make_pair(fenceValue, page));
    }
}

void LinearAllocatorPagePool::Clear() {
    std::lock_guard lock(mutex_);
    pagePool_.clear();
    ReadyPageQueue().swap(directReadyPageQueue_);
    ReadyPageQueue().swap(computeReadyPageQueue_);
    ReadyPageQueue().swap(copyReadyPageQueue_);
}

bool LinearAllocatorPagePool::TryAllocate(D3D12_COMMAND_LIST_TYPE commandType, LinearAllocatorPage** page) {
    // タイプのあったキューを持ってくる
    auto& readyPageQueue = GetReadyPageQueue(commandType);
    // キューが空ではない
    if (!readyPageQueue.empty()) {
        const auto& [fenceValue, readyPage] = readyPageQueue.front();
        auto& queue = Graphics::GetInstance()->GetCommandManager().GetCommandQueue();
        // GPUで使用していない
        if (fenceValue <= queue.GetLastCompletedFenceValue()) {
            readyPageQueue.pop();
            *page = readyPage;
            return true;
        }
    }
    // 失敗
    return false;
}

LinearAllocatorPage* LinearAllocatorPagePool::CreateNewPage() {
    auto newPage = pagePool_.emplace_back(std::make_unique<LinearAllocatorPage>()).get();
    std::wstringstream name;
#ifdef _DEBUG
    name << L"LinearAllocatorPage " << type_.GetName() << " " << pagePool_.size() - 1;
#endif // _DEBUG
    newPage->Create(name.str(), type_);

    return newPage;
}

void LinearAllocatorPage::Create(const std::wstring& name, LinearAllocatorType type) {
    size_ = type.GetSize();
    CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size_);
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_GENERIC_READ;
    if (type == LinearAllocatorType::Default) {
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        state = D3D12_RESOURCE_STATE_COMMON;
    }
    CreateResource(name, heapProps, desc, state);

    gpuAddressStart_ = resource_->GetGPUVirtualAddress();
    if (type == LinearAllocatorType::Upload) {
        ASSERT_IF_FAILED(resource_->Map(0, nullptr, &cpuAddressStart_));
    }
}

void LinearAllocator::Create(LinearAllocatorType type) {
    type_ = type;
}

LinearAllocator::Allocation LinearAllocator::Allocate(size_t size, size_t alignment) {
    std::lock_guard<std::mutex> lock(mutex_);
    assert(size <= type_.GetSize());

    if (!HasSpace(size, alignment)) {
        currentPage_ = Graphics::GetInstance()->GetLinearAllocatorPagePool(type_).Allocate();
        currentOffset_ = 0;
        usedPages_.emplace_back(currentPage_);
    }

    size_t alignedSize = Helper::AlignUp(size, alignment);
    currentOffset_ = Helper::AlignUp(currentOffset_, alignment);

    Allocation allocation{
        .resource = *currentPage_,
        .cpu = static_cast<uint8_t*>(currentPage_->GetCPUAddressStart()) + currentOffset_,
        .gpu = currentPage_->GetGPUAddressStart() + currentOffset_,
        .size = currentPage_->GetSize(),
        .offset = currentOffset_
    };
    currentOffset_ += alignedSize;

    return allocation;
}

void LinearAllocator::Reset(D3D12_COMMAND_LIST_TYPE commandType, UINT64 fenceValue) {
    std::lock_guard<std::mutex> lock(mutex_);
    currentPage_ = nullptr;
    currentOffset_ = 0;
    if (!usedPages_.empty()) {
        Graphics::GetInstance()->GetLinearAllocatorPagePool(type_).Discard(commandType, fenceValue, usedPages_);
        usedPages_.clear();
    }
}

bool LinearAllocator::HasSpace(size_t size, size_t alignment) {
    if (currentPage_ == nullptr) { return false; }
    size_t alignedSize = Helper::AlignUp(size, alignment);
    size_t alignedOffset = Helper::AlignUp(currentOffset_, alignment);

    return alignedOffset + alignedSize <= currentPage_->GetSize();
}
