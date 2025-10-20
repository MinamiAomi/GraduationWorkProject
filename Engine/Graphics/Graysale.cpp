#include "Graysale.h"

#include "Core/CommandContext.h"
#include "Core/ShaderManager.h"
#include "Core/ColorBuffer.h"

namespace {
    const wchar_t kComputeShader[] = L"GrayscaleCS.hlsl";
}

void Grayscale::Initialize() {
    CD3DX12_DESCRIPTOR_RANGE drs[1]{};
    drs[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

    CD3DX12_ROOT_PARAMETER rps[2]{};
    rps[0].InitAsDescriptorTable(1, drs + 0);
    rps[1].InitAsConstants(3, 0);

    D3D12_ROOT_SIGNATURE_DESC rsDesc{};
    rsDesc.pParameters = rps;
    rsDesc.NumParameters = _countof(rps);
    rootSignature_.Create(L"Grayscale RootSignature", rsDesc);

    auto cs = ShaderManager::GetInstance()->Compile(kComputeShader, ShaderType::Compute, 6, 0);
    D3D12_COMPUTE_PIPELINE_STATE_DESC cps{};
    cps.pRootSignature = rootSignature_;
    cps.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
    pipelineState_.Create(L"Grayscale PipelineState", cps);
}

void Grayscale::Dispatch(CommandContext& commandContext, ColorBuffer& texture) {
    commandContext.TransitionResource(texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.SetComputeRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetComputeDescriptorTable(0, texture.GetUAV());
    commandContext.SetComputeConstants(1, color_.x, color_.y, color_.z);
    commandContext.Dispatch((UINT)std::floor(texture.GetWidth() / 8), (UINT)std::floor(texture.GetHeight() / 8));
    commandContext.UAVBarrier(texture);

}
