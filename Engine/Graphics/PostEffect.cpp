#include "PostEffect.h"

#include "Core/Helper.h"
#include "Core/ShaderManager.h"
#include "Core/CommandContext.h"
#ifdef ENABLE_IMGUI
#include "ImGuiManager.h"
#endif ENABLE_IMGUI

namespace {
    const wchar_t kPostEffectVertexShader[] = L"ScreenQuadVS.hlsl";
    const wchar_t kPostEffectPixelShader[] = L"PostEffectPS.hlsl";
};

void PostEffect::Initialize(const ColorBuffer& target) {
    CD3DX12_DESCRIPTOR_RANGE srvRange[1]{};
    srvRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[2]{};
    rootParameters[0].InitAsConstantBufferView(0);
    rootParameters[1].InitAsDescriptorTable(1, &srvRange[0]);

    CD3DX12_STATIC_SAMPLER_DESC staticSampler[1]{};
    staticSampler->Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumStaticSamplers = _countof(staticSampler);
    rootSignatureDesc.pStaticSamplers = staticSampler;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    rootSignature_.Create(L"PostEffect RootSignature", rootSignatureDesc);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
    pipelineStateDesc.pRootSignature = rootSignature_;

    auto shaderManager = ShaderManager::GetInstance();
    auto vs = shaderManager->Compile(kPostEffectVertexShader, ShaderManager::kVertex);
    auto ps = shaderManager->Compile(kPostEffectPixelShader, ShaderManager::kPixel);
    pipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
    pipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
    pipelineStateDesc.BlendState = Helper::BlendDisable;
    pipelineStateDesc.RasterizerState = Helper::RasterizerNoCull;
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = target.GetRTVFormat();
    pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc.SampleDesc.Count = 1;

    pipelineState_.Create(L"PostEffect PipelineState", pipelineStateDesc);
}

void PostEffect::Render(CommandContext& commandContext, ColorBuffer& texture) {

    struct Constant {
        Vector3 grayscaleColor;
        uint32_t useGrayscale;
        float vignetteIntensity;
        float vignettePower;
        uint32_t useVignette;
        uint32_t pad1;
        Vector3 hsvBias;
        uint32_t pad2;
        Vector3 hsvFactor;
    };

    Constant constant;
    constant.grayscaleColor = grayscale_.color;
    constant.useGrayscale = grayscale_.isActive ? 1 : 0;
    constant.vignetteIntensity = vignette_.intensity;
    constant.vignettePower = vignette_.power;
    constant.useVignette = vignette_.isActive ? 1 : 0;
    constant.hsvBias = hsvFilter_.bias;
    constant.hsvFactor = hsvFilter_.factor;

    commandContext.TransitionResource(texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandContext.SetRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandContext.SetDynamicConstantBufferView(0, sizeof(constant), &constant);
    commandContext.SetDescriptorTable(1, texture.GetSRV());
    commandContext.Draw(3);
}

void PostEffect::DrawImGui(const char* label) {
    label;
#ifdef ENABLE_IMGUI
    if (ImGui::TreeNode(label)) {
        ImGui::Checkbox("##GrayscaleCheck", &grayscale_.isActive);
        ImGui::SameLine();
        if (ImGui::TreeNode("Grayscale")) {
            ImGui::ColorEdit3("Color", &grayscale_.color.x);
            ImGui::TreePop();
        }
        ImGui::Checkbox("##VignetteCheck", &vignette_.isActive);
        ImGui::SameLine();
        if (ImGui::TreeNode("Vignette")) {
            ImGui::DragFloat("Intensity", &vignette_.intensity, 0.1f);
            ImGui::DragFloat("Power", &vignette_.power, 0.01f);
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
#endif ENABLE_IMGUI
}

