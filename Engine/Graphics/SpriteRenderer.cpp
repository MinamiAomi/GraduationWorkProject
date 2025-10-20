#include "SpriteRenderer.h"

#include "Core/Helper.h"
#include "Core/ColorBuffer.h"
#include "Core/CommandContext.h"
#include "Core/ShaderManager.h"
#include "Core/SamplerManager.h"

#include "DefaultTextures.h"
#include "Sprite.h"

const wchar_t kSpriteVertexShadedr[] = L"SpriteVS.hlsl";
const wchar_t kSpritePixelShadedr[] = L"SpritePS.hlsl";

struct SpriteRootIndex {
    enum Index {
        Scene = 0,
        Texture,
        Sampler,

        NumParameters
    };
};

void SpriteRenderer::Initialize(const ColorBuffer& colorBuffer) {
    CD3DX12_DESCRIPTOR_RANGE srvRange[1]{};
    srvRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_DESCRIPTOR_RANGE samplerRange{};
    samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[SpriteRootIndex::NumParameters]{};
    rootParameters[SpriteRootIndex::Scene].InitAsConstantBufferView(0);
    rootParameters[SpriteRootIndex::Texture].InitAsDescriptorTable(1, &srvRange[0]);
    rootParameters[SpriteRootIndex::Sampler].InitAsDescriptorTable(1, &samplerRange);

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignature_.Create(L"Sprite RootSignature", rootSignatureDesc);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

    pipelineStateDesc.pRootSignature = rootSignature_;

    D3D12_INPUT_ELEMENT_DESC inputElements[] = {
         { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    D3D12_INPUT_LAYOUT_DESC inputLayout{};
    inputLayout.NumElements = _countof(inputElements);
    inputLayout.pInputElementDescs = inputElements;
    pipelineStateDesc.InputLayout = inputLayout;

    auto shaderManager = ShaderManager::GetInstance();
    auto vs = shaderManager->Compile(kSpriteVertexShadedr, ShaderManager::kVertex);
    auto ps = shaderManager->Compile(kSpritePixelShadedr, ShaderManager::kPixel);

    pipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
    pipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());

    pipelineStateDesc.BlendState = Helper::BlendAlpha;
    pipelineStateDesc.RasterizerState = Helper::RasterizerNoCull;
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = colorBuffer.GetFormat();
    pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineState_.Create(L"Sprite PipelineState", pipelineStateDesc);
}

void SpriteRenderer::Render(CommandContext& commandContext, float left, float top, float right, float bottom) {
    struct SceneConstant {
        Matrix4x4 orthoMatrix;
    };

    struct Vertex {
        Vector3 position;
        Vector2 texcoord;
        Vector4 color;
    };

    commandContext.SetRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    SceneConstant sceneConstant;
    sceneConstant.orthoMatrix = Matrix4x4::MakeOrthographicProjection(right - left, bottom - top, 0.0f, 1.0f);
    sceneConstant.orthoMatrix *= Matrix4x4::MakeScaling({ 1.0f, 1.0f, 1.0f });
    sceneConstant.orthoMatrix *= Matrix4x4::MakeTranslation({ -1.0f, -1.0f, 1.0f });
    commandContext.SetDynamicConstantBufferView(SpriteRootIndex::Scene, sizeof(sceneConstant), &sceneConstant);

    auto& instanceList = Sprite::instanceList_;
    // 描画順並べる
    instanceList.sort([](Sprite* a, Sprite* b) { return a->GetDrawOrder() < b->GetDrawOrder(); });

    for (auto instance : instanceList) {
        if (instance->isActive_) {

            float reciWidth = 1.0f;
            float reciHeight = 1.0f;
            if (instance->texture_) {
                reciWidth = 1.0f / (float)instance->texture_->GetWidth();
                reciHeight = 1.0f / (float)instance->texture_->GetHeight();
            }

            float uvLeft = instance->texcoordBase_.x * reciWidth;
            float uvRight = (instance->texcoordBase_.x + instance->texcoordSize_.x) * reciWidth;
            float uvTop = instance->texcoordBase_.y * reciHeight;
            float uvBottom = (instance->texcoordBase_.y + instance->texcoordSize_.y) * reciHeight;

            Vector2 localVertices[] = {
                { 0.0f, 0.0f },
                { 0.0f, 1.0f },
                { 1.0f, 1.0f },
                { 1.0f, 0.0f },
            };
            Matrix3x3 matrix = Matrix3x3::MakeTranslation(-instance->anchor_) * Matrix3x3::MakeAffineTransform(instance->scale_, instance->rotate_, instance->position_);

            for (auto& vertex : localVertices) {
                vertex = vertex * matrix;
            }

            Vertex vertices[] = {
                { { localVertices[0], 0.0f }, { uvLeft,  uvBottom}, instance->color_ },
                { { localVertices[1], 0.0f }, { uvLeft,  uvTop},    instance->color_ },
                { { localVertices[2], 0.0f }, { uvRight, uvTop},    instance->color_ },
                { { localVertices[0], 0.0f }, { uvLeft,  uvBottom}, instance->color_ },
                { { localVertices[2], 0.0f }, { uvRight, uvTop},    instance->color_ },
                { { localVertices[3], 0.0f }, { uvRight, uvBottom}, instance->color_ },

            };

            commandContext.SetDynamicVertexBuffer(0, 6, sizeof(vertices[0]), vertices);
            if (instance->texture_) {
                commandContext.SetDescriptorTable(SpriteRootIndex::Texture, instance->texture_->GetResource()->GetSRV());
                commandContext.SetDescriptorTable(SpriteRootIndex::Sampler, instance->texture_->GetSampler());
            }
            else {
                commandContext.SetDescriptorTable(SpriteRootIndex::Texture, DefaultTexture::White.GetSRV());
                commandContext.SetDescriptorTable(SpriteRootIndex::Sampler, SamplerManager::PointClamp);
            }
            commandContext.Draw(6);
        }
    }

}
