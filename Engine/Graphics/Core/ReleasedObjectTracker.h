#pragma once

#include <d3d12.h>  
#include <wrl/client.h>

#include <array>
#include <mutex>
#include <vector>
#include <string>

#include "DescriptorHandle.h"
#include "DescriptorHeap.h"

class ReleasedObjectTracker {
public:
    static const UINT kAliveFrameCount = 4;

    void AddObject(Microsoft::WRL::ComPtr<ID3D12Object> releasedObject);
    void AddDescriptor(uint32_t index, const std::shared_ptr<DescriptorHeap>& heap);
    void FrameIncrementForRelease();
    void AllRelease();

private:
    struct ReleasedObject {
        Microsoft::WRL::ComPtr<ID3D12Object> ptr;
#ifdef _DEBUG
        std::wstring name;
#endif // _DEBUG
    };

    struct ReleasedDescriptor {
        uint32_t index;
        std::weak_ptr<DescriptorHeap> heap;
        ~ReleasedDescriptor();
    };

    using TrackingObjectList = std::vector<ReleasedObject>;
    using TrackingDescriptorList = std::vector<ReleasedDescriptor>;
    
    std::array<TrackingObjectList, kAliveFrameCount> trackingObjectLists_;
    std::array<TrackingDescriptorList, kAliveFrameCount> trackingDescriptorLists_;
    std::mutex mutex_;
};
 