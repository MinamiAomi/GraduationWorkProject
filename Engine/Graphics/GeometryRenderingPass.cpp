#include "GeometryRenderingPass.h"

#include "Core/Helper.h"
#include "Core/Graphics.h"
#include "Core/ShaderManager.h"
#include "Core/CommandContext.h"
#include "Math/Camera.h"
#include "Core/SamplerManager.h"
#include "Model.h"
#include "DefaultTextures.h"
#include "RenderManager.h"

namespace {
    const wchar_t kVertexShader[] = L"Standard/GeometryPassVS.hlsl";
    const wchar_t kPixelShader[] = L"Standard/GeometryPassPS.hlsl";
}

void GeometryRenderingPass::Initialize(uint32_t width, uint32_t height) {

    float albedoClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    gBuffers_[GBuffer::Albedo].SetClearColor(albedoClearColor);
    gBuffers_[GBuffer::Albedo].Create(L"GeometryRenderingPass Albedo", width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
    float metallicRoughnessClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    gBuffers_[GBuffer::MetallicRoughness].SetClearColor(metallicRoughnessClearColor);
    gBuffers_[GBuffer::MetallicRoughness].Create(L"GeometryRenderingPass MetallicRoughness", width, height, DXGI_FORMAT_R16G16_UNORM);
    float normalClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    gBuffers_[GBuffer::Normal].SetClearColor(normalClearColor);
    gBuffers_[GBuffer::Normal].Create(L"GeometryRenderingPass Normal", width, height, DXGI_FORMAT_R10G10B10A2_UNORM);
    float viewDepthClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    gBuffers_[GBuffer::ViewDepth].SetClearColor(viewDepthClearColor);
    gBuffers_[GBuffer::ViewDepth].Create(L"GeometryRenderingPass ViewDepth", width, height, DXGI_FORMAT_R32_FLOAT);
    float meshMaterialIDsClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    gBuffers_[GBuffer::MeshMaterialIDs].SetClearColor(meshMaterialIDsClearColor);
    gBuffers_[GBuffer::MeshMaterialIDs].Create(L"GeometryRenderingPass MeshMaterialIDs", width, height, DXGI_FORMAT_R16G16_UINT);
    depth_.Create(L"GeometryRenderingPass Depth", width, height, DXGI_FORMAT_D32_FLOAT);

    RootSignatureDescHelper rootSignatureDesc;
    rootSignatureDesc.AddConstantBufferView(0);
    rootSignatureDesc.AddConstantBufferView(1);
    rootSignatureDesc.AddConstantBufferView(2);
    rootSignatureDesc.AddDescriptorTable().AddSRVDescriptors(BINDLESS_RESOURCE_MAX, 0, 1);
    rootSignatureDesc.AddStaticSampler(0, D3D12_FILTER_ANISOTROPIC);
    rootSignatureDesc.AddFlag(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    rootSignature_.Create(L"GeometryRenderingPass RootSignature", rootSignatureDesc);

    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
        pipelineStateDesc.pRootSignature = rootSignature_;

        D3D12_INPUT_ELEMENT_DESC inputElements[] = {
           { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
           { "NORMAL", 0, DXGI_FORMAT_R10G10B10A2_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
           { "TANGENT", 0, DXGI_FORMAT_R10G10B10A2_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
           { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
        pipelineStateDesc.DepthStencilState = Helper::DepthStateReadWrite;
        pipelineStateDesc.RasterizerState = Helper::RasterizerDefault;
        pipelineStateDesc.NumRenderTargets = GBuffer::NumGBuffers;
        for (uint32_t i = 0; i < GBuffer::NumGBuffers; ++i) {
            pipelineStateDesc.RTVFormats[i] = gBuffers_[i].GetRTVFormat();
        }
        pipelineStateDesc.DSVFormat = depth_.GetFormat();
        pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
        pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipelineStateDesc.SampleDesc.Count = 1;

        pipelineState_.Create(L"GeometryRenderingPass PipelineState", pipelineStateDesc);
    }

}

void GeometryRenderingPass::Render(CommandContext& commandContext, const Camera& camera, const ModelSorter& modelSorter) {

    struct SceneData {
        Matrix4x4 viewMatrix;
        Matrix4x4 projectionMatrix;
        Vector3 cameraPosition;
        float nearClip;
        float farClip;
    };

    struct InstanceData {
        Matrix4x4 worldMatrix;
        Matrix4x4 worldInverseTransposeMatrix;
    };


    struct MaterialData {
        Vector3 albedo;
        float metallic;
        float roughness;
        uint32_t albedoMapIndex;
        uint32_t metallicRoughnessMapIndex;
        uint32_t normalMapIndex;
    };

    uint32_t defaultWhiteTextureIndex = DefaultTexture::White.GetSRV().GetIndex();
    uint32_t defaultNormalTextureIndex = DefaultTexture::Normal.GetSRV().GetIndex();

    auto ErrorMaterial = [defaultWhiteTextureIndex, defaultNormalTextureIndex]() {
        MaterialData materialData;
        materialData.albedo = { 0.988f, 0.059f, 0.753f };
        materialData.metallic = 0.0f;
        materialData.roughness = 0.0f;
        materialData.albedoMapIndex = defaultWhiteTextureIndex;
        materialData.metallicRoughnessMapIndex = defaultWhiteTextureIndex;
        materialData.normalMapIndex = defaultNormalTextureIndex;
        return materialData;
        };
    auto SetMaterialData = [](MaterialData& dest, const Material& src) {
        dest.albedo = src.albedo;
        dest.metallic = src.metallic;
        dest.roughness = src.roughness;
        if (src.albedoMap) { dest.albedoMapIndex = src.albedoMap->GetSRV().GetIndex(); }
        if (src.metallicRoughnessMap) { dest.metallicRoughnessMapIndex = src.metallicRoughnessMap->GetSRV().GetIndex(); }
        if (src.normalMap) { dest.normalMapIndex = src.normalMap->GetSRV().GetIndex(); }
        };

    commandContext.BeginEvent(L"GeometryRenderingPass::Render");


    for (uint32_t i = 0; i < GBuffer::NumGBuffers; ++i) {
        commandContext.TransitionResource(gBuffers_[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
    commandContext.TransitionResource(depth_, D3D12_RESOURCE_STATE_DEPTH_WRITE);

    for (uint32_t i = 0; i < GBuffer::NumGBuffers; ++i) {
        commandContext.ClearColor(gBuffers_[i]);
    }
    commandContext.ClearDepth(depth_);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[GBuffer::NumGBuffers] = {};
    for (uint32_t i = 0; i < GBuffer::NumGBuffers; ++i) {
        rtvs[i] = gBuffers_[i].GetRTV();
    }

    commandContext.SetRenderTargets(_countof(rtvs), rtvs, depth_.GetDSV());
    commandContext.SetViewportAndScissorRect(0, 0, gBuffers_[0].GetWidth(), gBuffers_[0].GetHeight());
    commandContext.SetRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    SceneData sceneData;
    sceneData.viewMatrix = camera.GetViewMatrix();
    sceneData.projectionMatrix = camera.GetProjectionMatrix();
    sceneData.cameraPosition = camera.GetPosition();
    sceneData.nearClip = camera.GetNearClip();
    sceneData.farClip = camera.GetFarClip();
    commandContext.SetDynamicConstantBufferView(RootIndex::Scene, sizeof(sceneData), &sceneData);

    commandContext.SetBindlessResource(RootIndex::BindlessTexture);

    auto& instances = modelSorter.GetDrawModels();
    for (auto instance : instances) {
        auto model = instance->GetModel();

        InstanceData instanceData;
        instanceData.worldMatrix = /*model->GetRootNode().localMatrix **/ instance->GetWorldMatrix();
        instanceData.worldInverseTransposeMatrix = instanceData.worldMatrix.Inverse().Transpose();
        commandContext.SetDynamicConstantBufferView(RootIndex::Instance, sizeof(instanceData), &instanceData);

        auto instanceMaterial = instance->GetMaterial();

        for (auto& mesh : model->GetMeshes()) {
            MaterialData materialData = ErrorMaterial();
            // インスタンスのマテリアルを優先
            if (instanceMaterial) {
                SetMaterialData(materialData, *instanceMaterial);
            }
            // メッシュのマテリアル
            else if (mesh.material < model->GetMaterials().size()) {
                SetMaterialData(materialData, model->GetMaterials()[mesh.material]);
            }
            commandContext.SetDynamicConstantBufferView(RootIndex::Material, sizeof(materialData), &materialData);

            auto skeleton = instance->GetSkeleton();
            auto vbv = model->GetVertexBuffer().GetVertexBufferView();
            if (skeleton) {
                auto skinCluster = RenderManager::GetInstance()->GetSkinningManager().GetSkinCluster(skeleton.get());
                if (skinCluster) {
                    vbv = skinCluster->GetSkinnedVertexBuffer().GetVertexBufferView();
                }
            }
            commandContext.SetVertexBuffer(0, vbv);
            commandContext.SetIndexBuffer(model->GetIndexBuffer().GetIndexBufferView());
            commandContext.DrawIndexed((UINT)mesh.indexCount, mesh.indexOffset, mesh.vertexOffset);
        }

    }
    commandContext.EndEvent();
}
