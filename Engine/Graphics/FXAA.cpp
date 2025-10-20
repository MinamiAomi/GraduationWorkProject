#include "FXAA.h"

#include <cassert>
#include "Core/CommandContext.h"
#include "Core/ColorBuffer.h"
#include "Core/ShaderManager.h"

namespace {
    const wchar_t kVertexShader[] = L"ScreenQuadVS.hlsl";
    const wchar_t kPixelShader[] = L"FXAAPS.hlsl";
}

void FXAA::Initialize(ColorBuffer* original) {
    assert(original);
    original_ = original;

    result_.Create(L"FXAA Result", original_->GetWidth(), original_->GetHeight(), original_->GetFormat());

    CD3DX12_DESCRIPTOR_RANGE drs[1]{};
    drs[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_ROOT_PARAMETER rps[1]{};
    rps[0].InitAsDescriptorTable(1, drs + 0);

    CD3DX12_STATIC_SAMPLER_DESC ssd[1]{};
    ssd[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

    D3D12_ROOT_SIGNATURE_DESC rsDesc{};
    rsDesc.pParameters = rps;
    rsDesc.NumParameters = _countof(rps);
    rsDesc.pStaticSamplers = ssd;
    rsDesc.NumStaticSamplers = _countof(ssd);
    rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignature_.Create(L"FXAA RootSignature", rsDesc);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psDesc{};
    psDesc.pRootSignature = rootSignature_;

    auto shaderManager = ShaderManager::GetInstance();
    auto vs = shaderManager->Compile(kVertexShader, ShaderManager::kVertex);
    auto ps = shaderManager->Compile(kPixelShader, ShaderManager::kPixel);
    psDesc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
    psDesc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
    psDesc.BlendState = Helper::BlendDisable;
    psDesc.RasterizerState = Helper::RasterizerNoCull;
    psDesc.NumRenderTargets = 1;
    psDesc.RTVFormats[0] = result_.GetRTVFormat();
    psDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psDesc.SampleDesc.Count = 1;
    pipelineState_.Create(L"FXAA PipelineState", psDesc);
}

void FXAA::Render(CommandContext& commandContext) {
    assert(original_);
    commandContext.BeginEvent(L"FXAA::Render");
    commandContext.TransitionResource(result_, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandContext.TransitionResource(*original_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandContext.SetRenderTarget(result_.GetRTV());
    commandContext.SetViewportAndScissorRect(0, 0, result_.GetWidth(), result_.GetHeight());
    commandContext.SetRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandContext.SetDescriptorTable(0, original_->GetSRV());
    commandContext.Draw(3);
    commandContext.EndEvent();
}
