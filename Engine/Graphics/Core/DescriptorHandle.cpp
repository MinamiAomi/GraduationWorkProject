#include "DescriptorHandle.h"

#include "DescriptorHeap.h"
#include "Graphics.h"

DescriptorHandle::DescriptorHandle() :
    cpu_(D3D12_CPU_DESCRIPTOR_HANDLE_NULL),
    gpu_(D3D12_GPU_DESCRIPTOR_HANDLE_NULL),
    index_(0) {
}

DescriptorHandle::DescriptorHandle(DescriptorHandle&& move) :
    cpu_(move.cpu_),
    gpu_(move.gpu_),
    index_(move.index_),
    heap_(move.heap_) {
    move.cpu_ = D3D12_CPU_DESCRIPTOR_HANDLE_NULL;
    move.gpu_ = D3D12_GPU_DESCRIPTOR_HANDLE_NULL;
    move.index_ = 0;
    move.heap_.reset();
}

DescriptorHandle& DescriptorHandle::operator=(DescriptorHandle&& move) {
    Free();

    cpu_ = move.cpu_;
    gpu_ = move.gpu_;
    index_ = move.index_;
    heap_ = move.heap_;

    move.cpu_ = D3D12_CPU_DESCRIPTOR_HANDLE_NULL;
    move.gpu_ = D3D12_GPU_DESCRIPTOR_HANDLE_NULL;
    move.index_ = 0;
    move.heap_.reset();

    return *this;
}

DescriptorHandle::~DescriptorHandle() {
    Free();

}

void DescriptorHandle::Free() {
    auto ptr = heap_.lock();
    if (ptr) {
        Graphics::GetInstance()->GetReleasedObjectTracker().AddDescriptor(index_, ptr);
    }
    heap_.reset();
}
