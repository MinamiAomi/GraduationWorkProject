#include "TLAS.h"

#include <vector>

#include "../Model.h"
#include "../Core/CommandContext.h"
#include "../Core/Graphics.h"

void TLAS::Create(const std::wstring& name, CommandContext& commandContext, const D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs, size_t numInstanceDescs) {

    auto graphics = Graphics::GetInstance();


    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS asInputs{};
    asInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    asInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
    asInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    asInputs.NumDescs = UINT(numInstanceDescs);
    asInputs.InstanceDescs = commandContext.TransfarUploadBuffer(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * numInstanceDescs, instanceDescs);

    // ASのサイズを取得
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO asInfo{};
    graphics->GetDXRDevoce()->GetRaytracingAccelerationStructurePrebuildInfo(&asInputs, &asInfo);

    if (asInfo.ResultDataMaxSizeInBytes > reservedSize_) {
        CD3DX12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC resultDesc = CD3DX12_RESOURCE_DESC::Buffer(asInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        CreateResource(name, defaultHeapProps, resultDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
        reservedSize_ = asInfo.ResultDataMaxSizeInBytes;

        // ビューを生成
        CreateView();
    }

    if (asInfo.ScratchDataSizeInBytes > scratchResourceReservedSize_) {
        CD3DX12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC resultDesc = CD3DX12_RESOURCE_DESC::Buffer(asInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        scratchResource_.CreateResource(L"TLAS Scratch Resrouce" + name, defaultHeapProps, resultDesc, D3D12_RESOURCE_STATE_COMMON);
        
        scratchResourceReservedSize_ = asInfo.ScratchDataSizeInBytes;
    }

    // スクラッチリソース一時的なリソース
   // auto scratchAllocation = commandContext.AllocateDynamicBuffer(LinearAllocatorType::Default, asInfo.ScratchDataSizeInBytes);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc{};
    asDesc.Inputs = asInputs;
    asDesc.DestAccelerationStructureData = resource_->GetGPUVirtualAddress();
    asDesc.ScratchAccelerationStructureData = scratchResource_->GetGPUVirtualAddress();

    commandContext.TransitionResource(scratchResource_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.FlushResourceBarriers();
    commandContext.GetDXRCommandList()->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

    // 生成完了までUAVバリアを張る
    commandContext.UAVBarrier(*this);
    commandContext.UAVBarrier(scratchResource_);
    commandContext.FlushResourceBarriers();
    // スクラッチリソースが解放されないようにする

}

void TLAS::CreateView() {
    auto graphics = Graphics::GetInstance();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.RaytracingAccelerationStructure.Location = GetGPUVirtualAddress();

    if (srvHandle_.IsNull()) {
        srvHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    graphics->GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, srvHandle_);
}
