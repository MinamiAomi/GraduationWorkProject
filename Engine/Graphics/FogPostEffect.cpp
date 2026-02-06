#include "FogPostEffect.h"

#include "Core/CommandContext.h"
#include "Core/ShaderManager.h"
#include "Core/ColorBuffer.h"
#include "Core/DepthBuffer.h"

namespace {
    const wchar_t kComputeShader[] = L"FogPostEffectCS.hlsl";
}

void FogPostEffect::Initialize() {
    CD3DX12_DESCRIPTOR_RANGE drsUAV[1]{};
    drsUAV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

    CD3DX12_DESCRIPTOR_RANGE drsSRV[1]{};
    drsSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);


    CD3DX12_ROOT_PARAMETER rps[3]{};
    rps[0].InitAsDescriptorTable(1, drsUAV);
    rps[1].InitAsDescriptorTable(1, drsSRV);
    rps[2].InitAsConstantBufferView(0);

    D3D12_ROOT_SIGNATURE_DESC rsDesc{};
    rsDesc.pParameters = rps;
    rsDesc.NumParameters = _countof(rps);
    rootSignature_.Create(L"FogPostEffect RootSignature", rsDesc);

    auto cs = ShaderManager::GetInstance()->Compile(kComputeShader, ShaderType::Compute, 6, 0);
    D3D12_COMPUTE_PIPELINE_STATE_DESC cps{};
    cps.pRootSignature = rootSignature_;
    cps.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
    pipelineState_.Create(L"FogPostEffect PipelineState", cps);
}

void FogPostEffect::Dispatch(CommandContext& commandContext, ColorBuffer& texture, DepthBuffer& depth, const Camera& camera) {
    struct Constant {
        Matrix4x4 viewProjectionInverseMatrix;
        Vector3 cameraPosition;
        float pad;
        float depthFogStart;
        float depthFogEnd;
        float hightFogStart;
        float hightFogEnd;
        Vector3 color;
    };
    Constant constant;
    constant.viewProjectionInverseMatrix = camera.GetViewProjectionInverseMatrix();
    constant.cameraPosition = camera.GetPosition();
    constant.depthFogStart = depthFogStart_;
    constant.depthFogEnd = depthFogEnd_;
    constant.hightFogStart = hightFogStart_;
    constant.hightFogEnd = hightFogEnd_;
    constant.color = color_;

    commandContext.TransitionResource(texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.TransitionResource(depth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    commandContext.FlushResourceBarriers();

    commandContext.SetComputeRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetComputeDescriptorTable(0, texture.GetUAV());
    commandContext.SetComputeDescriptorTable(1, depth.GetSRV());
    commandContext.SetComputeDynamicConstantBufferView(2, sizeof(constant), &constant);
    commandContext.Dispatch((UINT)std::floor(texture.GetWidth() / 8), (UINT)std::floor(texture.GetHeight() / 8));
    commandContext.UAVBarrier(texture);

    commandContext.FlushResourceBarriers();
}
