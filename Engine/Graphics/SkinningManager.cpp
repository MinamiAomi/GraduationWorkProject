#include "SkinningManager.h"

#include "Core/ShaderManager.h"
#include "Core/CommandContext.h"

namespace {
    const wchar_t kComputeShader[] = L"Standard/SkinningCS.hlsl";
}

void SkinningManager::Initialize() {
    CD3DX12_DESCRIPTOR_RANGE matrixPaletteRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    CD3DX12_DESCRIPTOR_RANGE inputVerticesRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    CD3DX12_DESCRIPTOR_RANGE vertexInfluenceRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
    CD3DX12_DESCRIPTOR_RANGE outputVerticesRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[kNumRootParameters]{};
    rootParameters[kMatrixPalette].InitAsDescriptorTable(1, &matrixPaletteRange);
    rootParameters[kInputVertices].InitAsDescriptorTable(1, &inputVerticesRange);
    rootParameters[kVertexInfluence].InitAsDescriptorTable(1, &vertexInfluenceRange);
    rootParameters[kOutputVertices].InitAsDescriptorTable(1, &outputVerticesRange);
    rootParameters[kSkinningInformation].InitAsConstants(1, 0);

    D3D12_ROOT_SIGNATURE_DESC rsDesc{};
    rsDesc.pParameters = rootParameters;
    rsDesc.NumParameters = _countof(rootParameters);
    rootSignature_.Create(L"SkinningManager RootSignature", rsDesc);

    auto cs = ShaderManager::GetInstance()->Compile(kComputeShader, ShaderType::Compute, 6, 0);
    D3D12_COMPUTE_PIPELINE_STATE_DESC cps{};
    cps.pRootSignature = rootSignature_;
    cps.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
    pipelineState_.Create(L"SkinningManager PipelineState", cps);
}

void SkinningManager::Add(Skeleton* skeleton, const std::shared_ptr<Model>& model) {
    skinClusters_[skeleton] = std::make_unique<SkinCluster>();
    CommandContext commandContext;
    commandContext.Start(D3D12_COMMAND_LIST_TYPE_DIRECT);
    skinClusters_[skeleton]->Create(commandContext, model, *skeleton);
    commandContext.Finish(true);
}

void SkinningManager::Remove(Skeleton* skeleton) {
    auto it = skinClusters_.find(skeleton);
    if (it != skinClusters_.end()) {
        skinClusters_.erase(it);
    }
}

void SkinningManager::Update(CommandContext& commandContext) {

    if (skinClusters_.empty()) { return; }

    commandContext.SetComputeRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    for (auto& iter : skinClusters_) {
        auto skeleton = iter.first;
        if (!skeleton->updated_) { continue; }
        auto skinCluster = iter.second.get();
        auto model = skinCluster->model_.get();
        uint32_t numVertices = skinCluster->GetNumVertices();

        skinCluster->Update(commandContext, *skeleton);

        commandContext.TransitionResource(skinCluster->skinnedVertexBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        commandContext.SetComputeDescriptorTable(kMatrixPalette, skinCluster->matrixPaletteBuffer_.GetSRV());
        commandContext.SetComputeDescriptorTable(kInputVertices, model->GetVertexBuffer().GetSRV());
        commandContext.SetComputeDescriptorTable(kVertexInfluence, skinCluster->vertexInfluenceBuffer_.GetSRV());
        commandContext.SetComputeDescriptorTable(kOutputVertices, skinCluster->skinnedVertexBuffer_.GetUAV());
        commandContext.SetComputeConstants(kSkinningInformation, numVertices);
        commandContext.Dispatch((UINT)((numVertices + 1023) / 1024));
        commandContext.UAVBarrier(skinCluster->skinnedVertexBuffer_);


        commandContext.TransitionResource(skinCluster->skinnedVertexBuffer_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        commandContext.FlushResourceBarriers();
        skinCluster->skinnedBLAS_.Update(commandContext, skinCluster->blasDescs_);
        commandContext.TransitionResource(skinCluster->skinnedVertexBuffer_, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        commandContext.FlushResourceBarriers();
        skeleton->updated_ = false;
    }
}
