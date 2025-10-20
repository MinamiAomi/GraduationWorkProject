#include "SpatialDenoiser.h"

#include "../Core/ShaderManager.h"
#include "../Core/Helper.h"
#include "../Core/CommandContext.h"
#include "../Core/ColorBuffer.h"

namespace {
    const wchar_t kShader[] = L"Raytracing/Pathtracing/SpatialDenoiserCS.hlsl";
}

void SpatialDenoiser::Initialize(uint32_t width, uint32_t height, DXGI_FORMAT format) {
    float c[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    denoisedBuffer_.SetClearColor(c);
    denoisedBuffer_.Create(L"SpatialDenoiser DenoisedBuffer", width, height, format);

    RootSignatureDescHelper rsDesc;
    rsDesc.AddDescriptorTable().AddSRVDescriptors(1, 0);
    rsDesc.AddDescriptorTable().AddUAVDescriptors(1, 0);

    rsDesc.AddDescriptorTable().AddSRVDescriptors(1, 0, 1);
    rsDesc.AddDescriptorTable().AddSRVDescriptors(1, 1, 1);
    rsDesc.AddDescriptorTable().AddSRVDescriptors(1, 2, 1);
    rsDesc.AddDescriptorTable().AddSRVDescriptors(1, 3, 1);

    rsDesc.AddConstantBufferView(0);
    
    rootSignature_.Create(L"SpatialDenoiser", rsDesc);

    D3D12_COMPUTE_PIPELINE_STATE_DESC cpsDesc{};
    cpsDesc.pRootSignature = rootSignature_;
    auto cs = ShaderManager::GetInstance()->Compile(kShader, ShaderType::Compute, 6, 6);
    cpsDesc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
    pipelineState_.Create(L"SpatialDenoiser", cpsDesc);
}

void SpatialDenoiser::Dispatch(CommandContext& commandContext, ColorBuffer& sourceBuffer, GeometryRenderingPass& gBuffers) {
    struct Common {
        float albedoSigma;
        float normalSigma;
        float depthSigma;
    };

    Common common{};
    common.albedoSigma = settings_.albedoSigma;
    common.normalSigma = settings_.normalSigma;
    common.depthSigma = settings_.depthSigma;

    commandContext.BeginEvent(L"SpatialDenoiser::Dispatch");
    commandContext.TransitionResource(sourceBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    commandContext.TransitionResource(denoisedBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.SetComputeRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetComputeDescriptorTable(0, sourceBuffer.GetSRV());
    commandContext.SetComputeDescriptorTable(1, denoisedBuffer_.GetUAV());
    commandContext.SetComputeDescriptorTable(2, gBuffers.GetGBuffer(GBuffer::Albedo).GetSRV());
    commandContext.SetComputeDescriptorTable(3, gBuffers.GetGBuffer(GBuffer::MetallicRoughness).GetSRV());
    commandContext.SetComputeDescriptorTable(4, gBuffers.GetGBuffer(GBuffer::Normal).GetSRV());
    commandContext.SetComputeDescriptorTable(5, gBuffers.GetGBuffer(GBuffer::ViewDepth).GetSRV());
    commandContext.SetComputeDynamicConstantBufferView(6, sizeof(common), &common);
    commandContext.Dispatch(UINT((denoisedBuffer_.GetWidth() + 31) / 32), UINT((denoisedBuffer_.GetHeight() + 31) / 32));
    commandContext.UAVBarrier(denoisedBuffer_);
    commandContext.FlushResourceBarriers();
    commandContext.EndEvent();
}

