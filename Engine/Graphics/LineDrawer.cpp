#include "LineDrawer.h"

#include "Core/Helper.h"
#include "Core/ShaderManager.h"
#include "Core/ColorBuffer.h"
#include "Core/DepthBuffer.h"
#include "Core/CommandContext.h"

namespace {
    const wchar_t kVertexShader[] = L"LineVS.hlsl";
    const wchar_t kPixelShader[] = L"LinePS.hlsl";
}

void LineDrawer::Initialize(DXGI_FORMAT rtvFormat) {
    CD3DX12_ROOT_PARAMETER rootParameters[1]{};
    rootParameters[0].InitAsConstantBufferView(0);

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignature_.Create(L"LineDrawer RootSignature", rootSignatureDesc);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

    pipelineStateDesc.pRootSignature = rootSignature_;

    D3D12_INPUT_ELEMENT_DESC inputElements[] = {
         { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    D3D12_INPUT_LAYOUT_DESC inputLayout{};
    inputLayout.NumElements = _countof(inputElements);
    inputLayout.pInputElementDescs = inputElements;
    pipelineStateDesc.InputLayout = inputLayout;

    auto shaderManager = ShaderManager::GetInstance();
    auto vs = shaderManager->Compile(kVertexShader, ShaderManager::kVertex);
    auto ps = shaderManager->Compile(kPixelShader, ShaderManager::kPixel);

    pipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
    pipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());

    pipelineStateDesc.BlendState = Helper::BlendDisable;
    pipelineStateDesc.DepthStencilState = Helper::DepthStateDisabled;
    pipelineStateDesc.RasterizerState = Helper::RasterizerNoCull;
    // 前面カリング
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = rtvFormat;
    pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineState_.Create(L"LineDrawer PipelineState", pipelineStateDesc);

}

void LineDrawer::AddLine(const Vector3& start, const Vector3& end, const Vector4& color) {
    vertices_.emplace_back(start, color);
    vertices_.emplace_back(end, color);
}

void LineDrawer::Render(CommandContext& commandContext, const Camera& camera) {
    assert(vertices_.size() % 2 == 0);
    if (vertices_.empty()) { return; }
    commandContext.SetRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    Matrix4x4 viewProjectionMatrix = camera.GetViewProjectionMatrix();
    commandContext.SetDynamicConstantBufferView(0, sizeof(viewProjectionMatrix), &viewProjectionMatrix);
    commandContext.SetDynamicVertexBuffer(0, vertices_.size(), sizeof(vertices_[0]), vertices_.data());
    commandContext.Draw((UINT)vertices_.size());
    vertices_.clear();
}
