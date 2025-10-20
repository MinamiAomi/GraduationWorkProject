#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cassert>
#include <vector>
#include <string>

#include "GPUResource.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "LinearAllocator.h"
#include "UploadBuffer.h"
#include "Helper.h"

#define DXR_GRAPHICS_COMMAND_LIST ID3D12GraphicsCommandList4

// コマンドリストのラッパー
class CommandContext {
public:
    void Start(D3D12_COMMAND_LIST_TYPE type);
    void Close();
    UINT64 Finish(bool waitForCompletion);

    void TransitionResource(GPUResource& resource, D3D12_RESOURCE_STATES newState);
    void UAVBarrier(GPUResource& resource);
    void FlushResourceBarriers();

    void CopyBuffer(GPUResource& dest, GPUResource& src);
    void CopyBuffer(GPUResource& dest, size_t dataSize, const void* data);
    void CopyBufferRegion(GPUResource& dest, size_t destOffset, GPUResource& src, size_t srcOffset, size_t numBytes);

    void SetPipelineState(const PipelineState& pipelineState);
    void SetRootSignature(const RootSignature& rootSignature);
    void SetComputeRootSignature(const RootSignature& rootSignature);

    void ClearColor(ColorBuffer& target);
    void ClearColor(ColorBuffer& target, float clearColor[4]);
    void ClearDepth(DepthBuffer& target);
    void ClearDepth(DepthBuffer& target, float clearValue);

    void SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]);
    void SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE rtvs[], D3D12_CPU_DESCRIPTOR_HANDLE dsv);
    void SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE rtv) { SetRenderTargets(1, &rtv); }
    void SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv) { SetRenderTargets(1, &rtv, dsv); }

    void SetViewport(const D3D12_VIEWPORT& viewport);
    void SetViewport(FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT minDepth = 0.0f, FLOAT maxDepth = 1.0f);
    void SetScissorRect(const D3D12_RECT& rect);
    void SetScissorRect(UINT left, UINT top, UINT right, UINT bottom);
    void SetViewportAndScissorRect(const D3D12_VIEWPORT& viewport, const D3D12_RECT& rect);
    void SetViewportAndScissorRect(UINT x, UINT y, UINT w, UINT h);

    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology);

    void SetConstantArray(UINT rootIndex, UINT numConstants, const void* constants);
    void SetConstant(UINT rootIndex, UINT offset, Helper::DWParam value);
    void SetConstants(UINT rootIndex, Helper::DWParam x);
    void SetConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y);
    void SetConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y, Helper::DWParam z);
    void SetConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y, Helper::DWParam z, Helper::DWParam w);
    void SetConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address);
    void SetDescriptorTable(UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor);
    void SetBindlessResource(UINT rootIndex);
    void SetBindlessSampler(UINT rootIndex);

    void SetComputeConstantArray(UINT rootIndex, UINT numConstants, const void* constants);
    void SetComputeConstant(UINT rootIndex, UINT offset, Helper::DWParam value);
    void SetComputeConstants(UINT rootIndex, Helper::DWParam x);
    void SetComputeConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y);
    void SetComputeConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y, Helper::DWParam z);
    void SetComputeConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y, Helper::DWParam z, Helper::DWParam w);
    void SetComputeConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address);
    void SetComputeDescriptorTable(UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor);
    void SetComputeBindlessResource(UINT rootIndex);
    void SetComputeBindlessSampler(UINT rootIndex);

    void SetVertexBuffer(UINT slot, const D3D12_VERTEX_BUFFER_VIEW& vbv);
    void SetVertexBuffer(UINT slot, UINT numViews, const D3D12_VERTEX_BUFFER_VIEW vbvs[]);
    void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& ibv);

    void SetDynamicConstantBufferView(UINT rootIndex, size_t bufferSize, const void* bufferData);
    void SetDynamicShaderResourceView(UINT rootIndex, size_t bufferSize, const void* bufferData);
    void SetDynamicVertexBuffer(UINT slot, size_t numVertices, size_t vertexStride, const void* vertexData);
    void SetDynamicIndexBuffer(size_t numIndices, DXGI_FORMAT indexFormat, const void* indexData);

    void SetComputeDynamicConstantBufferView(UINT rootIndex, size_t bufferSize, const void* bufferData);
    void SetComputeDynamicShaderResourceView(UINT rootIndex, size_t bufferSize, const void* bufferData);

    void Draw(UINT vertexCount, UINT vertexStartOffset = 0);
    void DrawIndexed(UINT indexCount, UINT startIndexLocation = 0, INT baseVertexLocation = 0);
    void DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation = 0, UINT startInstanceLocation = 0);
    void DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation = 0, INT baseVertexLocation = 0, UINT startInstanceLocation = 0);

    void Dispatch(UINT threadGroupCountX);
    void Dispatch(UINT threadGroupCountX, UINT threadGroupCountY);
    void Dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ);

    D3D12_GPU_VIRTUAL_ADDRESS TransfarUploadBuffer(size_t bufferSize, const void* bufferData);

    LinearAllocator::Allocation AllocateDynamicBuffer(LinearAllocatorType type, size_t bufferSize, size_t alignment = 256);

    operator ID3D12GraphicsCommandList* () const { return commandList_.Get(); }

    DXR_GRAPHICS_COMMAND_LIST* GetDXRCommandList() const { return dxrCommandList_.Get(); }

    void SetMarker(const std::wstring& label);
    void BeginEvent(const std::wstring& label);
    void EndEvent();

private:
    static const uint32_t kMaxNumResourceBarriers = 16;

    D3D12_COMMAND_LIST_TYPE type_;
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
    Microsoft::WRL::ComPtr<DXR_GRAPHICS_COMMAND_LIST> dxrCommandList_;

    D3D12_RESOURCE_BARRIER resourceBarriers_[kMaxNumResourceBarriers]{};
    uint32_t numResourceBarriers_ = 0;

    ID3D12RootSignature* rootSignature_;
    ID3D12PipelineState* pipelineState_;

    ID3D12DescriptorHeap* resourceHeap_;
    ID3D12DescriptorHeap* samplerHeap_;

    D3D12_PRIMITIVE_TOPOLOGY primitiveTopology_;

    LinearAllocator dynamicBuffers_[LinearAllocatorType::Count];

    bool isClose_;
};

inline void CommandContext::TransitionResource(GPUResource& resource, D3D12_RESOURCE_STATES newState) {
    auto oldState = resource.state_;

    if (oldState == D3D12_RESOURCE_STATE_GENERIC_READ && (newState & oldState) == newState) {
        return;
    }

    if (newState != oldState) {
        assert(numResourceBarriers_ < kMaxNumResourceBarriers);
        auto& barrier = resourceBarriers_[numResourceBarriers_++];
        barrier = D3D12_RESOURCE_BARRIER{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = resource;
        barrier.Transition.StateBefore = oldState;
        barrier.Transition.StateAfter = newState;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        resource.state_ = newState;
    }

    if (numResourceBarriers_ >= kMaxNumResourceBarriers) {
        FlushResourceBarriers();
    }
}

inline void CommandContext::UAVBarrier(GPUResource& resource) {
    assert(numResourceBarriers_ < kMaxNumResourceBarriers);

    auto& barrier = resourceBarriers_[numResourceBarriers_++];
    barrier = D3D12_RESOURCE_BARRIER{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.UAV.pResource = resource;

    if (numResourceBarriers_ >= kMaxNumResourceBarriers) {
        FlushResourceBarriers();
    }
}

inline void CommandContext::FlushResourceBarriers() {
    if (numResourceBarriers_ > 0) {
        commandList_->ResourceBarrier(numResourceBarriers_, resourceBarriers_);
        numResourceBarriers_ = 0;
    }
}

inline void CommandContext::CopyBuffer(GPUResource& dest, GPUResource& src) {
    TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST);
    TransitionResource(src, D3D12_RESOURCE_STATE_COPY_SOURCE);
    FlushResourceBarriers();
    commandList_->CopyResource(dest, src);
}

inline void CommandContext::CopyBuffer(GPUResource& dest, size_t bufferSize, const void* data) {
    assert(data);

    if (bufferSize <= LinearAllocatorType(LinearAllocatorType::Upload).GetSize()) {
        auto allocation = dynamicBuffers_[LinearAllocatorType::Upload].Allocate(bufferSize);
        memcpy(allocation.cpu, data, bufferSize);
        TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST);
        FlushResourceBarriers();
        commandList_->CopyBufferRegion(dest, 0, allocation.resource, allocation.offset, bufferSize);
        return;
    }

    UploadBuffer uploadBuffer;
    uploadBuffer.Create(L"UploadBuffer CopySource", bufferSize);
    uploadBuffer.Copy(data, bufferSize);
    CopyBuffer(dest, uploadBuffer);
}

inline void CommandContext::CopyBufferRegion(GPUResource& dest, size_t destOffset, GPUResource& src, size_t srcOffset, size_t numBytes) {
    TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST);
    TransitionResource(src, D3D12_RESOURCE_STATE_COPY_SOURCE);
    FlushResourceBarriers();
    commandList_->CopyBufferRegion(dest, destOffset, src, srcOffset, numBytes);
}

inline void CommandContext::SetPipelineState(const PipelineState& pipelineState) {
    ID3D12PipelineState* ps = pipelineState;
    if (pipelineState_ != ps) {
        pipelineState_ = ps;
        commandList_->SetPipelineState(pipelineState_);
    }
}
inline void CommandContext::SetRootSignature(const RootSignature& rootSignature) {
    ID3D12RootSignature* rs = rootSignature;
    if (rootSignature_ != rs) {
        rootSignature_ = rs;
        commandList_->SetGraphicsRootSignature(rootSignature_);
    }
}

inline void CommandContext::SetComputeRootSignature(const RootSignature& rootSignature) {
    ID3D12RootSignature* rs = rootSignature;
    if (rootSignature_ != rs) {
        rootSignature_ = rs;
        commandList_->SetComputeRootSignature(rootSignature_);
    }
}

inline void CommandContext::ClearColor(ColorBuffer& target) {
    FlushResourceBarriers();
    commandList_->ClearRenderTargetView(target.GetRTV(), target.GetClearColor(), 0, nullptr);
}

inline void CommandContext::ClearColor(ColorBuffer& target, float clearColor[4]) {
    FlushResourceBarriers();
    commandList_->ClearRenderTargetView(target.GetRTV(), clearColor, 0, nullptr);
}

inline void CommandContext::ClearDepth(DepthBuffer& target) {
    FlushResourceBarriers();
    commandList_->ClearDepthStencilView(target.GetDSV(), D3D12_CLEAR_FLAG_DEPTH, target.GetClearValue(), 0, 0, nullptr);
}

inline void CommandContext::ClearDepth(DepthBuffer& target, float clearValue) {
    FlushResourceBarriers();
    commandList_->ClearDepthStencilView(target.GetDSV(), D3D12_CLEAR_FLAG_DEPTH, clearValue, 0, 0, nullptr);
}

inline void CommandContext::SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]) {
    commandList_->OMSetRenderTargets(numRTVs, rtvs, FALSE, nullptr);
}

inline void CommandContext::SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE rtvs[], D3D12_CPU_DESCRIPTOR_HANDLE dsv) {
    commandList_->OMSetRenderTargets(numRTVs, rtvs, FALSE, &dsv);
}

inline void CommandContext::SetViewport(const D3D12_VIEWPORT& viewport) {
    commandList_->RSSetViewports(1, &viewport);
}

inline void CommandContext::SetViewport(FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT minDepth, FLOAT maxDepth) {
    D3D12_VIEWPORT viewport{};
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;
    viewport.Width = w;
    viewport.Height = h;
    viewport.MinDepth = minDepth;
    viewport.MaxDepth = maxDepth;
    SetViewport(viewport);
}

inline void CommandContext::SetScissorRect(const D3D12_RECT& rect) {
    assert(rect.left < rect.right && rect.top < rect.bottom);
    commandList_->RSSetScissorRects(1, &rect);
}

inline void CommandContext::SetScissorRect(UINT left, UINT top, UINT right, UINT bottom) {
    D3D12_RECT rect{};
    rect.left = LONG(left);
    rect.top = LONG(top);
    rect.right = LONG(right);
    rect.bottom = LONG(bottom);
    SetScissorRect(rect);
}

inline void CommandContext::SetViewportAndScissorRect(const D3D12_VIEWPORT& viewport, const D3D12_RECT& rect) {
    SetViewport(viewport);
    SetScissorRect(rect);
}

inline void CommandContext::SetViewportAndScissorRect(UINT x, UINT y, UINT w, UINT h) {
    SetViewport(float(x), float(y), float(w), float(h));
    SetScissorRect(x, y, x + w, y + h);
}

inline void CommandContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) {
    if (topology != primitiveTopology_) {
        commandList_->IASetPrimitiveTopology(topology);
        primitiveTopology_ = topology;
    }
}

inline void CommandContext::SetConstantArray(UINT rootIndex, UINT numConstants, const void* constants) {
    commandList_->SetGraphicsRoot32BitConstants(rootIndex, numConstants, constants, 0);
}

inline void CommandContext::SetConstant(UINT rootIndex, UINT offset, Helper::DWParam value) {
    commandList_->SetGraphicsRoot32BitConstant(rootIndex, value.v.u, offset);
}

inline void CommandContext::SetConstants(UINT rootIndex, Helper::DWParam x) {
    commandList_->SetGraphicsRoot32BitConstant(rootIndex, x.v.u, 0);
}

inline void CommandContext::SetConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y) {
    commandList_->SetGraphicsRoot32BitConstant(rootIndex, x.v.u, 0);
    commandList_->SetGraphicsRoot32BitConstant(rootIndex, y.v.u, 1);
}

inline void CommandContext::SetConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y, Helper::DWParam z) {
    commandList_->SetGraphicsRoot32BitConstant(rootIndex, x.v.u, 0);
    commandList_->SetGraphicsRoot32BitConstant(rootIndex, y.v.u, 1);
    commandList_->SetGraphicsRoot32BitConstant(rootIndex, z.v.u, 2);
}

inline void CommandContext::SetConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y, Helper::DWParam z, Helper::DWParam w) {
    commandList_->SetGraphicsRoot32BitConstant(rootIndex, x.v.u, 0);
    commandList_->SetGraphicsRoot32BitConstant(rootIndex, y.v.u, 1);
    commandList_->SetGraphicsRoot32BitConstant(rootIndex, z.v.u, 2);
    commandList_->SetGraphicsRoot32BitConstant(rootIndex, w.v.u, 3);
}

inline void CommandContext::SetConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address) {
    commandList_->SetGraphicsRootConstantBufferView(rootIndex, address);
}

inline void CommandContext::SetDescriptorTable(UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor) {
    commandList_->SetGraphicsRootDescriptorTable(rootIndex, baseDescriptor);
}

inline void CommandContext::SetBindlessResource(UINT rootIndex) {
    commandList_->SetGraphicsRootDescriptorTable(rootIndex, resourceHeap_->GetGPUDescriptorHandleForHeapStart());
}

inline void CommandContext::SetBindlessSampler(UINT rootIndex) {
    commandList_->SetGraphicsRootDescriptorTable(rootIndex, samplerHeap_->GetGPUDescriptorHandleForHeapStart());
}

inline void CommandContext::SetComputeConstantArray(UINT rootIndex, UINT numConstants, const void* constants) {
    commandList_->SetComputeRoot32BitConstants(rootIndex, numConstants, constants, 0);
}

inline void CommandContext::SetComputeConstant(UINT rootIndex, UINT offset, Helper::DWParam value) {
    commandList_->SetComputeRoot32BitConstant(rootIndex, value.v.u, offset);
}

inline void CommandContext::SetComputeConstants(UINT rootIndex, Helper::DWParam x) {
    commandList_->SetComputeRoot32BitConstant(rootIndex, x.v.u, 0);
}

inline void CommandContext::SetComputeConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y) {
    commandList_->SetComputeRoot32BitConstant(rootIndex, x.v.u, 0);
    commandList_->SetComputeRoot32BitConstant(rootIndex, y.v.u, 1);
}

inline void CommandContext::SetComputeConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y, Helper::DWParam z) {
    commandList_->SetComputeRoot32BitConstant(rootIndex, x.v.u, 0);
    commandList_->SetComputeRoot32BitConstant(rootIndex, y.v.u, 1);
    commandList_->SetComputeRoot32BitConstant(rootIndex, z.v.u, 2);
}

inline void CommandContext::SetComputeConstants(UINT rootIndex, Helper::DWParam x, Helper::DWParam y, Helper::DWParam z, Helper::DWParam w) {
    commandList_->SetComputeRoot32BitConstant(rootIndex, x.v.u, 0);
    commandList_->SetComputeRoot32BitConstant(rootIndex, y.v.u, 1);
    commandList_->SetComputeRoot32BitConstant(rootIndex, z.v.u, 2);
    commandList_->SetComputeRoot32BitConstant(rootIndex, w.v.u, 3);
}

inline void CommandContext::SetComputeConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address) {
    commandList_->SetComputeRootConstantBufferView(rootIndex, address);
}

inline void CommandContext::SetComputeDescriptorTable(UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor) {
    commandList_->SetComputeRootDescriptorTable(rootIndex, baseDescriptor);
}

inline void CommandContext::SetComputeBindlessResource(UINT rootIndex) {
    commandList_->SetComputeRootDescriptorTable(rootIndex, resourceHeap_->GetGPUDescriptorHandleForHeapStart());
}

inline void CommandContext::SetComputeBindlessSampler(UINT rootIndex) {
    commandList_->SetComputeRootDescriptorTable(rootIndex, samplerHeap_->GetGPUDescriptorHandleForHeapStart());
}

inline void CommandContext::SetVertexBuffer(UINT slot, const D3D12_VERTEX_BUFFER_VIEW& vbv) {
    commandList_->IASetVertexBuffers(slot, 1, &vbv);
}

inline void CommandContext::SetVertexBuffer(UINT slot, UINT numViews, const D3D12_VERTEX_BUFFER_VIEW vbvs[]) {
    commandList_->IASetVertexBuffers(slot, numViews, vbvs);
}

inline void CommandContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& ibv) {
    commandList_->IASetIndexBuffer(&ibv);
}

inline void CommandContext::SetDynamicConstantBufferView(UINT rootIndex, size_t bufferSize, const void* bufferData) {
    assert(bufferData);

    auto allocation = dynamicBuffers_[LinearAllocatorType::Upload].Allocate(bufferSize, 256);
    memcpy(allocation.cpu, bufferData, bufferSize);
    commandList_->SetGraphicsRootConstantBufferView(rootIndex, allocation.gpu);
}

inline void CommandContext::SetDynamicShaderResourceView(UINT rootIndex, size_t bufferSize, const void* bufferData) {
    assert(bufferData);

    auto allocation = dynamicBuffers_[LinearAllocatorType::Upload].Allocate(bufferSize, 256);
    memcpy(allocation.cpu, bufferData, bufferSize);
    commandList_->SetGraphicsRootShaderResourceView(rootIndex, allocation.gpu);
}

inline void CommandContext::SetDynamicVertexBuffer(UINT slot, size_t numVertices, size_t vertexStride, const void* vertexData) {
    assert(vertexData);

    size_t bufferSize = Helper::AlignUp(numVertices * vertexStride, 16);
    auto allocation = dynamicBuffers_[LinearAllocatorType::Upload].Allocate(bufferSize);
    memcpy(allocation.cpu, vertexData, bufferSize);
    D3D12_VERTEX_BUFFER_VIEW vbv{
        .BufferLocation = allocation.gpu,
        .SizeInBytes = UINT(bufferSize),
        .StrideInBytes = UINT(vertexStride)
    };
    TransitionResource(allocation.resource, D3D12_RESOURCE_STATE_GENERIC_READ);
    commandList_->IASetVertexBuffers(slot, 1, &vbv);
}

inline void CommandContext::SetDynamicIndexBuffer(size_t numIndices, DXGI_FORMAT indexFormat, const void* indexData) {
    assert(indexData);
    assert(indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT);

    size_t stride = (indexFormat == DXGI_FORMAT_R16_UINT) ? sizeof(uint16_t) : sizeof(uint32_t);
    size_t bufferSize = Helper::AlignUp(numIndices * stride, 16);
    auto allocation = dynamicBuffers_[LinearAllocatorType::Upload].Allocate(bufferSize);
    memcpy(allocation.cpu, indexData, bufferSize);
    D3D12_INDEX_BUFFER_VIEW ibv{
        .BufferLocation = allocation.gpu,
        .SizeInBytes = UINT(numIndices * stride),
        .Format = indexFormat
    };
    commandList_->IASetIndexBuffer(&ibv);
}

inline void CommandContext::SetComputeDynamicConstantBufferView(UINT rootIndex, size_t bufferSize, const void* bufferData) {
    assert(bufferData);

    auto allocation = dynamicBuffers_[LinearAllocatorType::Upload].Allocate(bufferSize, 256);
    memcpy(allocation.cpu, bufferData, bufferSize);
    commandList_->SetComputeRootConstantBufferView(rootIndex, allocation.gpu);
}

inline void CommandContext::SetComputeDynamicShaderResourceView(UINT rootIndex, size_t bufferSize, const void* bufferData) {
    assert(bufferData);

    auto allocation = dynamicBuffers_[LinearAllocatorType::Upload].Allocate(bufferSize, 256);
    memcpy(allocation.cpu, bufferData, bufferSize);
    commandList_->SetComputeRootShaderResourceView(rootIndex, allocation.gpu);
}

inline void CommandContext::Draw(UINT vertexCount, UINT vertexStartOffset) {
    DrawInstanced(vertexCount, 1, vertexStartOffset, 0);
}

inline void CommandContext::DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation) {
    DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
}

inline void CommandContext::DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation) {
    FlushResourceBarriers();
    commandList_->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

inline void CommandContext::DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation) {
    FlushResourceBarriers();
    commandList_->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

inline void CommandContext::Dispatch(UINT threadGroupCountX) {
    FlushResourceBarriers();
    commandList_->Dispatch(threadGroupCountX, 1, 1);
}

inline void CommandContext::Dispatch(UINT threadGroupCountX, UINT threadGroupCountY) {
    FlushResourceBarriers();
    commandList_->Dispatch(threadGroupCountX, threadGroupCountY, 1);
}

inline void CommandContext::Dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ) {
    FlushResourceBarriers();
    commandList_->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

inline D3D12_GPU_VIRTUAL_ADDRESS CommandContext::TransfarUploadBuffer(size_t bufferSize, const void* bufferData) {
    auto allocation = dynamicBuffers_[LinearAllocatorType::Upload].Allocate(bufferSize);
    memcpy(allocation.cpu, bufferData, bufferSize);
    return allocation.gpu;
}

inline LinearAllocator::Allocation CommandContext::AllocateDynamicBuffer(LinearAllocatorType type, size_t bufferSize, size_t alignment) {
    return dynamicBuffers_[type].Allocate(bufferSize, alignment);
}



