#include "GaussianBlur.h"

#include <cassert>
#include <cmath>

#include "Core/Helper.h"
#include "Core/ShaderManager.h"
#include "Core/CommandContext.h"

namespace {

    const char kGaussianBlurHorizontalVS[] = "HorizontalGaussianBlurVS.hlsl";
    const char kGaussianBlurVerticalVS[] = "VerticalGaussianBlurVS.hlsl";
    const char kGaussianBlurPS[] = "GaussianBlurPS.hlsl";

    struct PipelineSet {
        PipelineState horizontalBlurPSO;
        PipelineState verticalBlurPSO;
    };

    std::unique_ptr<RootSignature> gbRootSignature_;
    std::map<DXGI_FORMAT, std::unique_ptr<PipelineSet>> gbPipelineStateMap_;
    uint32_t gbInstanceCount = 0;

    void CreateRootSignature() {
        gbRootSignature_ = std::make_unique<RootSignature>();

        CD3DX12_DESCRIPTOR_RANGE range{};
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        CD3DX12_ROOT_PARAMETER rootParameters[2]{};
        rootParameters[0].InitAsDescriptorTable(1, &range);
        rootParameters[1].InitAsConstantBufferView(0);

        CD3DX12_STATIC_SAMPLER_DESC staticSamplerDesc(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.pParameters = rootParameters;
        rootSignatureDesc.NumParameters = _countof(rootParameters);
        rootSignatureDesc.pStaticSamplers = &staticSamplerDesc;
        rootSignatureDesc.NumStaticSamplers = 1;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        gbRootSignature_->Create(L"GaussianBlur RootSignature", rootSignatureDesc);
    }

    void CreatePipelineState(DXGI_FORMAT format) {
        if (!gbPipelineStateMap_.contains(format)) {
            auto psos = std::make_unique<PipelineSet>();

            auto shaderManager = ShaderManager::GetInstance();

            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
            psoDesc.pRootSignature = *gbRootSignature_;

            auto vs = shaderManager->Compile(kGaussianBlurHorizontalVS, ShaderManager::kVertex);
            auto ps = shaderManager->Compile(kGaussianBlurPS, ShaderManager::kPixel);
            psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
            psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());

            psoDesc.BlendState = Helper::BlendDisable;
            psoDesc.RasterizerState = Helper::RasterizerNoCull;
            psoDesc.NumRenderTargets = 1;
            psoDesc.RTVFormats[0] = format;
            psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            psoDesc.SampleDesc.Count = 1;

            psos->horizontalBlurPSO.Create(L"GaussianBlur HorizontalPSO", psoDesc);

            vs = shaderManager->Compile(kGaussianBlurVerticalVS, ShaderManager::kVertex);
            psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());

            psos->verticalBlurPSO.Create(L"GaussianBlur VerticalPSO", psoDesc);

            gbPipelineStateMap_[format] = std::move(psos);
        }
    }
}

GaussianBlur::GaussianBlur() {
    gbInstanceCount++;
}

GaussianBlur::~GaussianBlur() {
    assert(gbInstanceCount > 0);
    gbInstanceCount--;
    if (gbInstanceCount == 0) {
        gbRootSignature_.reset();
        gbPipelineStateMap_.clear();
    }
}

void GaussianBlur::Initialize(ColorBuffer* originalTexture) {
    assert(originalTexture);

    if (!gbRootSignature_) {
        CreateRootSignature();
    }

    originalTexture_ = originalTexture;
    horizontalBlurTexture_.Create(
        L"GaussianBlur HorizontalBlurTexture",
        originalTexture_->GetWidth() / 2,
        originalTexture_->GetHeight(),
        originalTexture_->GetFormat());
    verticalBlurTexture_.Create(
        L"GaussianBlur VerticalBlurTexture",
        originalTexture_->GetWidth() / 2,
        originalTexture_->GetHeight() / 2,
        originalTexture_->GetFormat());

    constantBuffer_.Create(L"GaussianBlur Constant", sizeof(weights_));

    CreatePipelineState(originalTexture->GetRTVFormat());

    UpdateWeightTable(1.0f);
}

void GaussianBlur::Render(CommandContext& commandContext) {

    auto pipelineSet = gbPipelineStateMap_[originalTexture_->GetFormat()].get();

    commandContext.TransitionResource(*originalTexture_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandContext.TransitionResource(horizontalBlurTexture_, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandContext.SetRenderTarget(horizontalBlurTexture_.GetRTV());
    commandContext.ClearColor(horizontalBlurTexture_);
    commandContext.SetViewportAndScissorRect(0, 0, horizontalBlurTexture_.GetWidth(), horizontalBlurTexture_.GetHeight());

    commandContext.SetRootSignature(*gbRootSignature_);
    commandContext.SetPipelineState(pipelineSet->horizontalBlurPSO);
    commandContext.SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandContext.SetDescriptorTable(0, originalTexture_->GetSRV());
    commandContext.SetConstantBuffer(1, constantBuffer_.GetGPUVirtualAddress());
    commandContext.Draw(3);

    commandContext.TransitionResource(horizontalBlurTexture_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandContext.TransitionResource(verticalBlurTexture_, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandContext.ClearColor(verticalBlurTexture_);
    commandContext.SetRenderTarget(verticalBlurTexture_.GetRTV());
    commandContext.SetViewportAndScissorRect(0, 0, verticalBlurTexture_.GetWidth(), verticalBlurTexture_.GetHeight());

    commandContext.SetRootSignature(*gbRootSignature_);
    commandContext.SetPipelineState(pipelineSet->verticalBlurPSO);
    commandContext.SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandContext.SetDescriptorTable(0, horizontalBlurTexture_.GetSRV());
    commandContext.SetConstantBuffer(1, constantBuffer_.GetGPUVirtualAddress());
    commandContext.Draw(3);

    commandContext.TransitionResource(verticalBlurTexture_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void GaussianBlur::UpdateWeightTable(float blurPower) {
    float total = 0;
    for (uint32_t i = 0; i < kNumWeights; ++i) {
        weights_[i] = std::exp(-0.5f * float(i * i) / blurPower);
        total += 2.0f * weights_[i];
    }
    total = 1.0f / total;
    for (uint32_t i = 0; i < kNumWeights; ++i) {
        weights_[i] *= total;
    }
    constantBuffer_.Copy(weights_, sizeof(weights_));
}
