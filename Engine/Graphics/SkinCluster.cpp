#include "SkinCluster.h"

#include <cassert>
#include <span>

#include "Core/CommandContext.h"

void SkinCluster::Create(CommandContext& commandContext, const std::shared_ptr<Model>& model, const Skeleton& skeleton) {
    assert(model);
    model_ = model;
    matrixPaletteBuffer_.Create(L"SkinCluster MatrixPallette", skeleton.GetJoints().size(), sizeof(Well));
    // サブメッシュも同じバッファに送る
    vertexInfluenceBuffer_.Create(L"SkinCluster VertexInfluence", model_->GetNumVertices(), sizeof(VertexInfluence));
    inverseBindPoseMatrices_.resize(skeleton.GetJoints().size());
    skinnedVertexBuffer_.Create(L"SkinCluster SkinnedVertex", model_->GetNumVertices(), sizeof(Model::Vertex));
    
    auto vertexInfluenceBufferAllocation = commandContext.AllocateDynamicBuffer(LinearAllocatorType::Upload, vertexInfluenceBuffer_.GetBufferSize());
    memset(vertexInfluenceBufferAllocation.cpu, 0, vertexInfluenceBuffer_.GetBufferSize());
    std::span<VertexInfluence> mappedInfluence = { reinterpret_cast<VertexInfluence*>(vertexInfluenceBufferAllocation.cpu), model_->GetNumVertices() };
   
    numVertices_ = (uint32_t)model_->GetNumVertices();
    auto& jointMap = skeleton.GetJointMap();

    for (auto& jointWeight : model_->GetSkinClusterData()) {
        auto it = jointMap.find(jointWeight.first);
        if (it == jointMap.end()) {
            continue;
        }

        inverseBindPoseMatrices_[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
        for (auto& vertexWeight : jointWeight.second.vertexWeights) {
            auto& currentInfluence = mappedInfluence[vertexWeight.vertexIndex];
            for (uint32_t index = 0; index < SkinCluster::kNumMaxInfluence; ++index) {
                if (currentInfluence.weights[index] == 0.0f) {
                    currentInfluence.weights[index] = vertexWeight.weight;
                    currentInfluence.jointIndices[index] = (*it).second;
                    break;
                }
            }
        }
    }
    
    commandContext.CopyBufferRegion(vertexInfluenceBuffer_, 0, vertexInfluenceBufferAllocation.resource, vertexInfluenceBufferAllocation.offset, vertexInfluenceBuffer_.GetBufferSize());
    commandContext.TransitionResource(vertexInfluenceBuffer_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    commandContext.TransitionResource(skinnedVertexBuffer_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    commandContext.FlushResourceBarriers();

    // レイトレ用にBLASを作成
    blasDescs_.resize(model->GetMeshes().size());
    for (uint32_t meshIndex = 0; meshIndex < blasDescs_.size(); ++meshIndex) {
        auto& mesh = model->GetMeshes()[meshIndex];
        auto& desc = blasDescs_[meshIndex];
        desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        desc.Triangles.VertexBuffer.StartAddress = skinnedVertexBuffer_.GetGPUVirtualAddress() + (uint64_t)mesh.vertexOffset * skinnedVertexBuffer_.GetElementSize();
        desc.Triangles.VertexBuffer.StrideInBytes = skinnedVertexBuffer_.GetElementSize();
        desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        desc.Triangles.VertexCount = mesh.vertexCount;
        desc.Triangles.IndexBuffer = model->GetIndexBuffer().GetGPUVirtualAddress() + (uint64_t)mesh.indexOffset * model->GetIndexBuffer().GetElementSize();
        desc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
        desc.Triangles.IndexCount = mesh.indexCount;

    }
    skinnedBLAS_.Create(L"Skinned BLAS", commandContext, blasDescs_, true);

    Update(commandContext, skeleton);
}
 
void SkinCluster::Update(CommandContext& commandContext, const Skeleton& skeleton) {
    assert(matrixPaletteBuffer_.GetBufferSize() == skeleton.GetJoints().size() * sizeof(Well));
    
    auto matrixPaletteBufferAllocation = commandContext.AllocateDynamicBuffer(LinearAllocatorType::Upload, matrixPaletteBuffer_.GetBufferSize());
    
    std::span<Well> mappedPalette = { reinterpret_cast<Well*>(matrixPaletteBufferAllocation.cpu), skeleton.GetJoints().size() };
    for (size_t jointIndex = 0; jointIndex < skeleton.GetJoints().size(); ++jointIndex) {
        assert(jointIndex < inverseBindPoseMatrices_.size());
        
        mappedPalette[jointIndex].skeletonSpaceMatrix = inverseBindPoseMatrices_[jointIndex] * skeleton.GetJoints()[jointIndex].skeletonSpaceMatrix;
        mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix = mappedPalette[jointIndex].skeletonSpaceMatrix.Inverse().Transpose();
    }
    commandContext.CopyBufferRegion(matrixPaletteBuffer_, 0, matrixPaletteBufferAllocation.resource, matrixPaletteBufferAllocation.offset, matrixPaletteBuffer_.GetBufferSize());
    commandContext.TransitionResource(matrixPaletteBuffer_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    commandContext.FlushResourceBarriers();
}
