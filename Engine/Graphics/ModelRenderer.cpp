#include "ModelRenderer.h"

#include "Core/Helper.h"
#include "Core/ShaderManager.h"
#include "Core/ColorBuffer.h"
#include "Core/DepthBuffer.h"
#include "Core/CommandContext.h"
#include "Math/Camera.h"
#include "Core/SamplerManager.h"
#include "Model.h"
#include "DefaultTextures.h"
#include "LightManager.h"

namespace {
    const wchar_t kVertexShader[] = L"Standard/StandardModelVS.hlsl";
    const wchar_t kPixelShader[] = L"Standard/StandardModelPS.hlsl";
}

void ModelRenderer::Initialize(const ColorBuffer& colorBuffer, const DepthBuffer& depthBuffer) {
    InitializeRootSignature();
    InitializePipelineState(colorBuffer.GetRTVFormat(), depthBuffer.GetFormat());
}

void ModelRenderer::Render(CommandContext& commandContext, const Camera& camera, const DirectionalLight& sunLight) {
    struct SceneConstant {
        Matrix4x4 viewMatrix;
        Matrix4x4 projectionMatrix;
        Vector3 cameraPosition;
        float pad0;
        Vector3 sunLightColor;
        float pad1;
        Vector3 sunLightDirection;
        float sunLightIntensity;
        //uint32_t numDirectionalLights;
        //uint32_t numPointLights;
        //uint32_t numSpotLights;
    };

    struct InstanceConstant {
        Matrix4x4 worldMatrix;
        Matrix4x4 worldInverseTransposeMatrix;
        Vector3 color;
        float alpha;
        uint32_t useLighting;
    };

    struct MaterialConstant {
        Vector3 diffuse = { 1.0f, 0.8f, 0.8f };
        float shininess = 0.0f;
        Vector3 specular = Vector3::zero;
    };

    //struct DirectionalLight {
    //    Vector3 direction;
    //    float intensity;
    //    Vector3 color;
    //};

    //struct PointLight {
    //    Vector3 position;
    //    float intensity;
    //    Vector3 color;
    //    float radius;
    //    float decay;
    //};

    //struct SpotLight {
    //    Vector3 direction;
    //    float intensity;
    //    Vector3 position;
    //    float distance;
    //    Vector3 color;
    //    float decay;
    //    float cosAngle;
    //    float cosFalloffStart;
    //};

    auto& instanceList = ModelInstance::GetInstanceList();


    // 描画
    commandContext.SetRootSignature(rootSignature_);
    commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


    SceneConstant scene{};
    scene.viewMatrix = camera.GetViewMatrix();
    scene.projectionMatrix = camera.GetProjectionMatrix();
    scene.cameraPosition = camera.GetPosition();
    scene.sunLightColor = sunLight.color;
    scene.sunLightDirection = sunLight.direction;
    scene.sunLightIntensity = sunLight.intensity;
    commandContext.SetDynamicConstantBufferView(RootIndex::Scene, sizeof(scene), &scene);

    /*{
        auto& srcDL = lightManager.GetDirectionalLights();
        std::vector<DirectionalLight> directionalLights;
        directionalLights.reserve(srcDL.size());

        for (const auto& light : srcDL) {
            if (light) {
                directionalLights.emplace_back(light->direction, light->intensity, light->color);
            }
        }
        commandContext.SetDynamicShaderResourceView(RootIndex::DirectionalLights, sizeof(DirectionalLights[0])* directionalLights.size(), directionalLights.data());
    }
    {
        auto& srcPL = lightManager.GetPointLights();
        std::vector<PointLight> pointLights;
        pointLights.reserve(srcPL.size());

        for (const auto& light : srcPL) {
            if (light) {
                pointLights.emplace_back(light->direction, light->intensity, light->color);
            }
        }
        commandContext.SetDynamicShaderResourceView(RootIndex::DirectionalLights, sizeof(DirectionalLights[0]) * directionalLights.size(), directionalLights.data());
    }
    {
        auto& srcDL = lightManager.GetDirectionalLights();
        std::vector<DirectionalLight> directionalLights;
        directionalLights.reserve(srcDL.size());

        for (const auto& light : srcDL) {
            if (light) {
                directionalLights.emplace_back(light->direction, light->intensity, light->color);
            }
        }
        commandContext.SetDynamicShaderResourceView(RootIndex::DirectionalLights, sizeof(DirectionalLights[0]) * directionalLights.size(), directionalLights.data());
    }*/

    for (auto& instance : instanceList) {
        if (instance->IsActive() && instance->GetModel()) {

            InstanceConstant data;
            data.worldMatrix = instance->GetWorldMatrix();
            data.worldInverseTransposeMatrix = instance->GetWorldMatrix().Inverse().Transpose();
            data.color = instance->GetColor();
            data.alpha = instance->GetAlpha();
            data.useLighting = instance->UseLighting() ? 1 : 0;
            commandContext.SetDynamicConstantBufferView(RootIndex::Instance, sizeof(data), &data);

            // オブジェクト描画
            commandContext.SetPipelineState(pipelineState_);

            //for (auto& mesh : instance->GetModel()->GetMeshes()) {
               // MaterialConstant material;
               // D3D12_GPU_DESCRIPTOR_HANDLE texture = DefaultTexture::White.GetSRV();
               // if (mesh.material) {
               //     material.diffuse = mesh.material->diffuse;
               //     material.shininess = mesh.material->shininess;
               //     material.specular = mesh.material->specular;
               //     if (mesh.material->diffuseMap) {
               //         texture = mesh.material->diffuseMap->GetSRV();
               //     }
               // }

               // commandContext.SetDynamicConstantBufferView(RootIndex::Material, sizeof(material), &material);
               // commandContext.SetDescriptorTable(RootIndex::Texture, texture);
               // commandContext.SetDescriptorTable(RootIndex::Sampler, SamplerManager::AnisotropicWrap);
               // commandContext.SetVertexBuffer(0, mesh.vertexBuffer.GetVertexBufferView());
               // commandContext.SetIndexBuffer(mesh.indexBuffer.GetIndexBufferView());
               // commandContext.DrawIndexed((UINT)mesh.indices.size());
            //}
        }
    }
}

void ModelRenderer::InitializeRootSignature() {
    CD3DX12_DESCRIPTOR_RANGE srvRange{};
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_DESCRIPTOR_RANGE samplerRange{};
    samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[RootIndex::NumParameters]{};
    rootParameters[RootIndex::Scene].InitAsConstantBufferView(0);
    rootParameters[RootIndex::Instance].InitAsConstantBufferView(1);
    rootParameters[RootIndex::Material].InitAsConstantBufferView(2);
    rootParameters[RootIndex::Texture].InitAsDescriptorTable(1, &srvRange);
    rootParameters[RootIndex::Sampler].InitAsDescriptorTable(1, &samplerRange);
    rootParameters[RootIndex::DirectionalLights].InitAsShaderResourceView(1);
    rootParameters[RootIndex::PointLights].InitAsShaderResourceView(2);
    rootParameters[RootIndex::SpotLights].InitAsShaderResourceView(3);

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignature_.Create(L"ModelRenderer RootSignature", rootSignatureDesc);
}

void ModelRenderer::InitializePipelineState(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

    pipelineStateDesc.pRootSignature = rootSignature_;

    D3D12_INPUT_ELEMENT_DESC inputElements[] = {
         { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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

    pipelineStateDesc.BlendState = Helper::BlendAlpha;
    pipelineStateDesc.DepthStencilState = Helper::DepthStateReadWrite;
    pipelineStateDesc.RasterizerState = Helper::RasterizerDefault;
    // 前面カリング
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = rtvFormat;
    pipelineStateDesc.DSVFormat = dsvFormat;
    pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineState_.Create(L"ModelRenderer PipelineState", pipelineStateDesc);
}
