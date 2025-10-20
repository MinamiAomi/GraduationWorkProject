#include "ReleasedObjectTracker.h"

#include <cassert>

void ReleasedObjectTracker::AddObject(Microsoft::WRL::ComPtr<ID3D12Object> releasedObject) {
    assert(releasedObject);
    std::lock_guard<std::mutex> lock(mutex_);
    ReleasedObject object;
    object.ptr = releasedObject;

#ifdef _DEBUG
    wchar_t name[256] = {};
    UINT size = sizeof(name);
    releasedObject->GetPrivateData(WKPDID_D3DDebugObjectNameW, &size, name);
    object.name = name;
#endif // _DEBUG

    auto& back = trackingObjectLists_.back();
    back.emplace_back(std::move(object));
}

void ReleasedObjectTracker::AddDescriptor(uint32_t index, const std::shared_ptr<DescriptorHeap>& heap) {
    assert(heap);
    std::lock_guard<std::mutex> lock(mutex_);
    ReleasedDescriptor descriptor;
    descriptor.index = index;
    descriptor.heap = heap;
    trackingDescriptorLists_.back().emplace_back(std::move(descriptor));
}

void ReleasedObjectTracker::FrameIncrementForRelease() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (size_t i = 1; i < trackingDescriptorLists_.size(); ++i) {
        std::swap(trackingDescriptorLists_[i - 1], trackingDescriptorLists_[i]);
    }
    for (size_t i = 1; i < trackingObjectLists_.size(); ++i) {
        std::swap(trackingObjectLists_[i - 1], trackingObjectLists_[i]);
    }
    trackingObjectLists_.back().clear();
    trackingDescriptorLists_.back().clear();
}

void ReleasedObjectTracker::AllRelease() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& descriptorHandleList : trackingDescriptorLists_) {
        descriptorHandleList.clear();
    }
    for (auto& trackingObjecetList : trackingObjectLists_) {
        trackingObjecetList.clear();
    }
}

ReleasedObjectTracker::ReleasedDescriptor::~ReleasedDescriptor() {
    auto ptr = heap.lock();
    if (ptr) {
        ptr->Free(index);
    }
}
