#include "ComputeShader.h"

#include "Core/CommandContext.h"
#include "Core/ShaderManager.h"

const wchar_t kComputeShader[] = L"TestCS.hlsl";

void ComputeShaderTester::Initialize(uint32_t width, uint32_t height) {
    CD3DX12_DESCRIPTOR_RANGE drs[1]{};
    drs[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

    CD3DX12_ROOT_PARAMETER rps[2]{};
    rps[0].InitAsDescriptorTable(1, drs + 0);
    rps[1].InitAsConstantBufferView(0);

    D3D12_ROOT_SIGNATURE_DESC rsDesc{};
    rsDesc.pParameters = rps;
    rsDesc.NumParameters = _countof(rps);
    rootSignature_.Create(L"ComputeShaderTester RootSignature", rsDesc);

    auto cs = ShaderManager::GetInstance()->Compile(kComputeShader, ShaderType::Compute, 6, 0);
    D3D12_COMPUTE_PIPELINE_STATE_DESC cps{};
    cps.pRootSignature = rootSignature_;
    cps.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
    pipelineState_.Create(L"ComputeShaderTester PipelineState", cps);

    texture_.Create(L"ComputeShaderTester ColorBuffer", width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
}


void ComputeShaderTester::Dispatch(CommandContext& commandContext) {
    static const float cycle = 120.0f;
    static float time = 0.0f;

    time += 1.0f / cycle;

    commandContext.TransitionResource(texture_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.SetComputeRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);

    commandContext.SetComputeDescriptorTable(0, texture_.GetUAV());
    commandContext.SetComputeDynamicConstantBufferView(1, sizeof(time), &time);

    commandContext.Dispatch((UINT)std::floor(texture_.GetWidth() / 8), (UINT)std::floor(texture_.GetHeight() / 8));
    commandContext.UAVBarrier(texture_);
    commandContext.TransitionResource(texture_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandContext.FlushResourceBarriers();
}
