#include "TemporalDenoiser.h"

#include "../Core/ShaderManager.h"
#include "../Core/Helper.h"
#include "../Core/CommandContext.h"

namespace {
    const wchar_t kShader[] = L"Raytracing/Pathtracing/TemporalDenoiserCS.hlsl";
}

void TemporalDenoiser::Initialize(uint32_t width, uint32_t height, DXGI_FORMAT format) {
    // 蓄積バッファの生成
    float c[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    denoisedBuffer_.SetClearColor(c);
    denoisedBuffer_.Create(L"TemporalDenoiser DenoisedBuffer", width, height, format);
    // ルートシグネチャの生成
    RootSignatureDescHelper rsDesc;
    // IntermadiateBuffer
    rsDesc.AddDescriptorTable().AddSRVDescriptors(1, 0);
    // AccumulationBuffer
    rsDesc.AddDescriptorTable().AddUAVDescriptors(1, 0);
    // DenoisedBuffer
    rsDesc.AddDescriptorTable().AddUAVDescriptors(1, 1);
    rsDesc.AddConstants(1, 0);
    rootSignature_.Create(L"TemporalDenoiser", rsDesc);
    // パイプラインの生成
    D3D12_COMPUTE_PIPELINE_STATE_DESC cpsDesc{};
    cpsDesc.pRootSignature = rootSignature_;
    auto cs = ShaderManager::GetInstance()->Compile(kShader, ShaderType::Compute, 6, 6);
    cpsDesc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
    pipelineState_.Create(L"TemporalDenoiser", cpsDesc);
    // カウンタをリセット
    sampleCount_ = 1;
}

void TemporalDenoiser::Dispatch(CommandContext& commandContext, ColorBuffer& intermediateBuffer) {
    assert(intermediateBuffer.GetWidth() == denoisedBuffer_.GetWidth());
    assert(intermediateBuffer.GetHeight() == denoisedBuffer_.GetHeight());
    commandContext.BeginEvent(L"TemporalDenoiser::Dispatch");
    commandContext.TransitionResource(intermediateBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    commandContext.TransitionResource(denoisedBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.SetComputeRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetComputeDescriptorTable(0, intermediateBuffer.GetSRV());
    commandContext.SetComputeDescriptorTable(1, denoisedBuffer_.GetUAV());
    commandContext.SetComputeConstants(3, sampleCount_++);
    commandContext.Dispatch(UINT((denoisedBuffer_.GetWidth() + 31) / 32), UINT((denoisedBuffer_.GetHeight() + 31) / 32));
    commandContext.UAVBarrier(denoisedBuffer_);
    commandContext.FlushResourceBarriers();
    commandContext.EndEvent();
}

void TemporalDenoiser::Reset(CommandContext& commandContext) {
    commandContext.BeginEvent(L"TemporalDenoiser::Reset");
    commandContext.TransitionResource(denoisedBuffer_, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandContext.ClearColor(denoisedBuffer_);
    sampleCount_ = 1;
    commandContext.EndEvent();    
}
