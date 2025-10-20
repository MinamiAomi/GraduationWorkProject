#include "Transition.h"

#include "Core/ColorBuffer.h"
#include "Core/CommandContext.h"
#include "Core/ShaderManager.h"

const wchar_t kComputeShader[] = L"TransitionCS.hlsl";

void Transition::Initialize() {
    CD3DX12_DESCRIPTOR_RANGE drs[1]{};
    drs[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

    CD3DX12_ROOT_PARAMETER rps[2]{};
    rps[0].InitAsDescriptorTable(1, drs + 0);
    rps[1].InitAsConstantBufferView(0);

    D3D12_ROOT_SIGNATURE_DESC rsDesc{};
    rsDesc.pParameters = rps;
    rsDesc.NumParameters = _countof(rps);
    rootSignature_.Create(L"Transition RootSignature", rsDesc);

    auto cs = ShaderManager::GetInstance()->Compile(kComputeShader, ShaderType::Compute, 6, 0);
    D3D12_COMPUTE_PIPELINE_STATE_DESC cps{};
    cps.pRootSignature = rootSignature_;
    cps.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
    pipelineState_.Create(L"Transition PipelineState", cps);
}

void Transition::Dispatch(CommandContext& commandContext, ColorBuffer& texture) {

    struct Parameter {
        float time;
        Vector3 fadeColor;
    };

    Parameter parameter;
    parameter.time = time_;
    parameter.fadeColor = fadeColor_;

    commandContext.TransitionResource(texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.SetComputeRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);

    commandContext.SetComputeDescriptorTable(0, texture.GetUAV());
    commandContext.SetComputeDynamicConstantBufferView(1, sizeof(parameter), &parameter);

    commandContext.Dispatch((UINT)std::ceil(texture.GetWidth() / 8), (UINT)std::ceil(texture.GetHeight() / 8));
    commandContext.UAVBarrier(texture);
    commandContext.FlushResourceBarriers();
}
