#include "FinalEffect.h"

#include "Core/CommandContext.h"
#include "Core/ShaderManager.h"
#include "Core/ColorBuffer.h"
#include "Core/DepthBuffer.h"
#ifdef ENABLE_IMGUI
#include "ImGuiManager.h"
#endif ENABLE_IMGUI

namespace {
    const wchar_t kComputeShader[] = L"FinalEffectCS.hlsl";
}

void FinalEffect::Initialize() {
    CD3DX12_DESCRIPTOR_RANGE drsUAV[1]{};
    drsUAV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);


    CD3DX12_ROOT_PARAMETER rps[1]{};
    rps[0].InitAsDescriptorTable(1, drsUAV);

    D3D12_ROOT_SIGNATURE_DESC rsDesc{};
    rsDesc.pParameters = rps;
    rsDesc.NumParameters = _countof(rps);
    rootSignature_.Create(L"FinalEffect RootSignature", rsDesc);

    auto cs = ShaderManager::GetInstance()->Compile(kComputeShader, ShaderType::Compute, 6, 0);
    D3D12_COMPUTE_PIPELINE_STATE_DESC cps{};
    cps.pRootSignature = rootSignature_;
    cps.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
    pipelineState_.Create(L"FinalEffect PipelineState", cps);
}

void FinalEffect::Dispatch(CommandContext& commandContext, ColorBuffer& texture) {

    commandContext.TransitionResource(texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.FlushResourceBarriers();

    commandContext.SetComputeRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetComputeDescriptorTable(0, texture.GetUAV());
    commandContext.Dispatch((UINT)std::floor(texture.GetWidth() / 8), (UINT)std::floor(texture.GetHeight() / 8));
    commandContext.UAVBarrier(texture);
    commandContext.TransitionResource(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandContext.FlushResourceBarriers();
}
