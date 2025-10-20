#include "Skybox.h"

#include "Core/Helper.h"
#include "Core/ShaderManager.h"
#include "Core/CommandContext.h"
#include "Math/Camera.h"

namespace {
    const wchar_t kVertexShader[] = L"SkyboxVS.hlsl";
    const wchar_t kPixelShader[] = L"SkyboxPS.hlsl";
}

void Skybox::Initialize(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat) {
    CD3DX12_DESCRIPTOR_RANGE srvDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    CD3DX12_ROOT_PARAMETER rootParameters[2]{};
    rootParameters[0].InitAsConstantBufferView(0);
    rootParameters[1].InitAsDescriptorTable(1, &srvDescriptorRange);
    D3D12_TEXTURE_ADDRESS_MODE textureAddressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    CD3DX12_STATIC_SAMPLER_DESC staticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, textureAddressMode, textureAddressMode, textureAddressMode);
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pStaticSamplers = &staticSampler;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignature_.Create(L"Skybox RootSignature", rootSignatureDesc);

    D3D12_INPUT_ELEMENT_DESC inputElements[] = {
         { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    auto shaderManager = ShaderManager::GetInstance();
    auto vs = shaderManager->Compile(kVertexShader, ShaderManager::kVertex);
    auto ps = shaderManager->Compile(kPixelShader, ShaderManager::kPixel);
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
    pipelineStateDesc.pRootSignature = rootSignature_;
    pipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
    pipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
    pipelineStateDesc.InputLayout.pInputElementDescs = inputElements;
    pipelineStateDesc.InputLayout.NumElements = _countof(inputElements);
    pipelineStateDesc.BlendState = Helper::BlendDisable;
    pipelineStateDesc.DepthStencilState = Helper::DepthStateReadOnly;
    pipelineStateDesc.RasterizerState = Helper::RasterizerDefault;
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = rtvFormat;
    pipelineStateDesc.DSVFormat = dsvFormat;
    pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineState_.Create(L"Skybox PipelineState", pipelineStateDesc);
}

void Skybox::Render(CommandContext& commandContext, const Camera& camera) {
    if (!texture_) { return; }

    struct Constant {
        Matrix4x4 worldViewProjectionMatrix;
        Vector4 color;
    } constant;
    constant.worldViewProjectionMatrix = worldMatrix_ * camera.GetViewProjectionMatrix();
    constant.color = color_;


    const Vector3 vertices[] = {
    // 右
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f,-1.0f },
        { 1.0f,-1.0f, 1.0f },

        { 1.0f,-1.0f, 1.0f },
        { 1.0f, 1.0f,-1.0f },
        { 1.0f,-1.0f,-1.0f },
    // 左
        {-1.0f, 1.0f,-1.0f },
        {-1.0f, 1.0f, 1.0f },
        {-1.0f,-1.0f,-1.0f },

        {-1.0f,-1.0f,-1.0f },
        {-1.0f, 1.0f, 1.0f },
        {-1.0f,-1.0f, 1.0f },
    // 前
        {-1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        {-1.0f,-1.0f, 1.0f },

        {-1.0f,-1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f,-1.0f, 1.0f },
    // 後
        { 1.0f, 1.0f,-1.0f },
        {-1.0f, 1.0f,-1.0f },
        { 1.0f,-1.0f,-1.0f },

        { 1.0f,-1.0f,-1.0f },
        {-1.0f, 1.0f,-1.0f },
        {-1.0f,-1.0f,-1.0f },
    // 上
        {-1.0f, 1.0f,-1.0f },
        { 1.0f, 1.0f,-1.0f },
        {-1.0f, 1.0f, 1.0f },

        {-1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f,-1.0f },
        { 1.0f, 1.0f, 1.0f },
    // 下
        {-1.0f,-1.0f, 1.0f },
        { 1.0f,-1.0f, 1.0f },
        {-1.0f,-1.0f,-1.0f },

        {-1.0f,-1.0f,-1.0f },
        { 1.0f,-1.0f, 1.0f },
        { 1.0f,-1.0f,-1.0f },
    };

    commandContext.SetRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandContext.SetDynamicConstantBufferView(0, sizeof(constant), &constant);
    commandContext.SetDescriptorTable(1, texture_->GetSRV());
    size_t numVertices = _countof(vertices);
    commandContext.SetDynamicVertexBuffer(0, numVertices, sizeof(vertices[0]), vertices);    
    commandContext.Draw((UINT)numVertices);
}
