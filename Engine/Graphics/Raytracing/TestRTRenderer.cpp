#include "TestRTRenderer.h"

#include <span>

#include "../Core/Graphics.h"
#include "../Core/Helper.h"
#include "../Core/ShaderManager.h"
#include "../Core/CommandContext.h"
#include "../Model.h"
#include "../DefaultTextures.h"
#include "../Core/SamplerManager.h"
#include "../ModelSorter.h"
#include "../RenderManager.h"
#include "../Shader/Raytracing/Pathtracing/Pathtracing.h"

#include "Framework/Engine.h"
#include "Input/Input.h"

namespace {
    static const wchar_t kRayGenerationShader[] = L"Raytracing/Pathtracing/RayGeneration.hlsl";
    static const wchar_t kClosestHitShader[] = L"Raytracing/Pathtracing/ClosestHit.hlsl";
    static const wchar_t kMissShader[] = L"Raytracing/Pathtracing/Miss.hlsl";
    static const wchar_t kRayGenerationName[] = L"RayGeneration";
    static const wchar_t kRecursiveMissName[] = L"RecursiveMiss";
    static const wchar_t kRecursiveClosestHitName[] = L"RecursiveClosestHit";
    static const wchar_t kRecursiveHitGroupName[] = L"RecursiveHitGroup";

    void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc) {
        std::wstringstream wstr;
        wstr << L"\n";
        wstr << L"--------------------------------------------------------------------\n";
        wstr << L"| D3D12 State Object 0x" << static_cast<const void*>(desc) << L": ";
        if (desc->Type == D3D12_STATE_OBJECT_TYPE_COLLECTION) wstr << L"Collection\n";
        if (desc->Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE) wstr << L"Raytracing Pipeline\n";

        auto ExportTree = [](UINT depth, UINT numExports, const D3D12_EXPORT_DESC* exports) {
            std::wostringstream woss;
            for (UINT i = 0; i < numExports; i++) {
                woss << L"|";
                if (depth > 0) {
                    for (UINT j = 0; j < 2 * depth - 1; j++) woss << L" ";
                }
                woss << L" [" << i << L"]: ";
                if (exports[i].ExportToRename) woss << exports[i].ExportToRename << L" --> ";
                woss << exports[i].Name << L"\n";
            }
            return woss.str();
        };

        for (UINT i = 0; i < desc->NumSubobjects; i++) {
            wstr << L"| [" << i << L"]: ";
            switch (desc->pSubobjects[i].Type) {
            case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
                wstr << L"Global Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
                wstr << L"Local Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
                wstr << L"Node Mask: 0x" << std::hex << std::setfill(L'0') << std::setw(8) << *static_cast<const UINT*>(desc->pSubobjects[i].pDesc) << std::setw(0) << std::dec << L"\n";
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY:
            {
                wstr << L"DXIL Library 0x";
                auto lib = static_cast<const D3D12_DXIL_LIBRARY_DESC*>(desc->pSubobjects[i].pDesc);
                wstr << lib->DXILLibrary.pShaderBytecode << L", " << lib->DXILLibrary.BytecodeLength << L" bytes\n";
                wstr << ExportTree(1, lib->NumExports, lib->pExports);
                break;
            }
            case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION:
            {
                wstr << L"Existing Library 0x";
                auto collection = static_cast<const D3D12_EXISTING_COLLECTION_DESC*>(desc->pSubobjects[i].pDesc);
                wstr << collection->pExistingCollection << L"\n";
                wstr << ExportTree(1, collection->NumExports, collection->pExports);
                break;
            }
            case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
            {
                wstr << L"Subobject to Exports Association (Subobject [";
                auto association = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
                UINT index = static_cast<UINT>(association->pSubobjectToAssociate - desc->pSubobjects);
                wstr << index << L"])\n";
                for (UINT j = 0; j < association->NumExports; j++) {
                    wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
                }
                break;
            }
            case D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
            {
                wstr << L"DXIL Subobjects to Exports Association (";
                auto association = static_cast<const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
                wstr << association->SubobjectToAssociate << L")\n";
                for (UINT j = 0; j < association->NumExports; j++) {
                    wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
                }
                break;
            }
            case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
            {
                wstr << L"Raytracing Shader Config\n";
                auto config = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG*>(desc->pSubobjects[i].pDesc);
                wstr << L"|  [0]: Max Payload Size: " << config->MaxPayloadSizeInBytes << L" bytes\n";
                wstr << L"|  [1]: Max Attribute Size: " << config->MaxAttributeSizeInBytes << L" bytes\n";
                break;
            }
            case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
            {
                wstr << L"Raytracing Pipeline Config\n";
                auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG*>(desc->pSubobjects[i].pDesc);
                wstr << L"|  [0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << L"\n";
                break;
            }
            case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
            {
                wstr << L"Hit Group (";
                auto hitGroup = static_cast<const D3D12_HIT_GROUP_DESC*>(desc->pSubobjects[i].pDesc);
                wstr << (hitGroup->HitGroupExport ? hitGroup->HitGroupExport : L"[none]") << L")\n";
                wstr << L"|  [0]: Any Hit Import: " << (hitGroup->AnyHitShaderImport ? hitGroup->AnyHitShaderImport : L"[none]") << L"\n";
                wstr << L"|  [1]: Closest Hit Import: " << (hitGroup->ClosestHitShaderImport ? hitGroup->ClosestHitShaderImport : L"[none]") << L"\n";
                wstr << L"|  [2]: Intersection Import: " << (hitGroup->IntersectionShaderImport ? hitGroup->IntersectionShaderImport : L"[none]") << L"\n";
                break;
            }
            }
            wstr << L"|--------------------------------------------------------------------\n";
        }
        wstr << L"\n";
        OutputDebugStringW(wstr.str().c_str());
    }
}

void TestRTRenderer::Create(uint32_t width, uint32_t height) {
    CreateRootSignature();
    CreateStateObject();
    CreateShaderTables();
    float c[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    intermediateBuffer_.SetClearColor(c);
    intermediateBuffer_.Create(L"TestRTRenderer IntermadiateBuffer", width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
    denoisedBuffer_.Create(L"TestRTRenderer DenoisedBuffer", width, height, DXGI_FORMAT_R11G11B10_FLOAT);
    time_ = 0;
    spatialDenoiser_.Initialize(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
    // 精度の高いフォーマットを使用
    temporalDenoiser_.Initialize(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
}

void TestRTRenderer::Render(CommandContext& commandContext, const Camera& camera, const ModelSorter& modelSorter) {
    modelSorter;
    struct SceneData {
        Matrix4x4 viewProjectionInverseMatrix;
        Vector3 cameraPosition;
        int32_t time;
        uint32_t skyboxLod;
    };


    auto commandList = commandContext.GetDXRCommandList();
    commandList;

    SceneData scene;
    scene.viewProjectionInverseMatrix = camera.GetViewProjectionMatrix().Inverse();
    scene.cameraPosition = camera.GetPosition();
    scene.time = ++time_;
    //scene.skyboxLod = (uint32_t)skyboxTexture_->GetDesc().MipLevels - 1;
    auto sceneCB = commandContext.TransfarUploadBuffer(sizeof(scene), &scene);
    sceneCB;

    commandContext.BeginEvent(L"TestRTRenderer");

    BuildScene(commandContext, modelSorter);


    commandContext.TransitionResource(intermediateBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.FlushResourceBarriers();

    commandContext.SetComputeRootSignature(globalRootSignature_);

    commandList->SetComputeRootConstantBufferView(0, sceneCB);
    commandList->SetComputeRootShaderResourceView(1, tlas_.GetGPUVirtualAddress());
    commandContext.SetComputeBindlessResource(2);
    commandList->SetComputeRootDescriptorTable(3, intermediateBuffer_.GetUAV());

    D3D12_DISPATCH_RAYS_DESC rayDesc{};
    rayDesc.RayGenerationShaderRecord.StartAddress = rayGenerationShaderTable_.GetGPUVirtualAddress();
    rayDesc.RayGenerationShaderRecord.SizeInBytes = rayGenerationShaderTable_.GetBufferSize();
    rayDesc.MissShaderTable.StartAddress = missShaderTable_.GetGPUVirtualAddress();
    rayDesc.MissShaderTable.SizeInBytes = missShaderTable_.GetBufferSize();
    rayDesc.MissShaderTable.StrideInBytes = missShaderTable_.GetShaderRecordSize();
    rayDesc.HitGroupTable.StartAddress = hitGroupShaderTable_.GetGPUVirtualAddress();
    rayDesc.HitGroupTable.SizeInBytes = hitGroupShaderTable_.GetBufferSize();
    rayDesc.HitGroupTable.StrideInBytes = hitGroupShaderTable_.GetShaderRecordSize();
    rayDesc.Width = intermediateBuffer_.GetWidth();
    rayDesc.Height = intermediateBuffer_.GetHeight();
    rayDesc.Depth = 1;
    commandList->SetPipelineState1(stateObject_);
    commandList->DispatchRays(&rayDesc);

    commandContext.UAVBarrier(intermediateBuffer_);
    commandContext.FlushResourceBarriers();

    commandContext.BeginEvent(L"Denoise Phase");
    //spatialDenoiser_.Dispatch(commandContext, intermediateBuffer_);
    
    // 仮リセット
    if (Engine::GetInput()->IsKeyPressed(DIK_R)) {
        temporalDenoiser_.Reset(commandContext);
    }

    temporalDenoiser_.Dispatch(commandContext, intermediateBuffer_);
    commandContext.EndEvent();
    commandContext.EndEvent();
    commandContext.SetMarker(L"DispatchRays");
}

void TestRTRenderer::CreateRootSignature() {

    RootSignatureDescHelper globalRootSignatureDesc;
    // Scene
    globalRootSignatureDesc.AddConstantBufferView(0);
    // TLAS
    globalRootSignatureDesc.AddShaderResourceView(0);
    // BindlessTexture
    globalRootSignatureDesc.AddDescriptorTable().AddSRVDescriptors(BINDLESS_RESOURCE_MAX, 0, 1);
    // Color
    globalRootSignatureDesc.AddDescriptorTable().AddUAVDescriptors(1, 0);
    globalRootSignatureDesc.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_POINT);
    globalRootSignatureDesc.AddStaticSampler(1, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
    globalRootSignatureDesc.SetFlag(D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED);
    globalRootSignature_.Create(L"GlobalRootSignature", globalRootSignatureDesc);


    RootSignatureDescHelper hitGroupLRSDesc;
    hitGroupLRSDesc.SetFlag(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
    // MeshProperties
    hitGroupLRSDesc.AddShaderResourceView(0, 2);
    hitGroupLocalRootSignature_.Create(L"HitGroupLocalRootSignature", hitGroupLRSDesc);

    RootSignatureDescHelper missLRSDesc;
    missLRSDesc.SetFlag(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
    missLRSDesc.AddConstantBufferView(0, 3);
    missLRSDesc.AddDescriptorTable().AddSRVDescriptors(1, 0, 3);
    missLRSDesc.AddDescriptorTable().AddSRVDescriptors(1, 1, 3);
    missLocalRootSignature_.Create(L"MissLocalRootSignature", missLRSDesc);
}

void TestRTRenderer::CreateStateObject() {
    CD3DX12_STATE_OBJECT_DESC stateObjectDesc{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    auto shaderManager = ShaderManager::GetInstance();

    std::vector<Microsoft::WRL::ComPtr<IDxcBlob>> shaderCashe;

    auto CreateShaderSubobject = [&](const wchar_t* shaderPath, const wchar_t* exportName) {
        auto shader = shaderManager->Compile(shaderPath, ShaderType::Library, 6, 6);
        shaderCashe.emplace_back(shader);
        CD3DX12_SHADER_BYTECODE shaderByteCode(shader->GetBufferPointer(), shader->GetBufferSize());
        auto dxilLibSubobject = stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
        dxilLibSubobject->SetDXILLibrary(&shaderByteCode);
        dxilLibSubobject->DefineExport(exportName);
    };

    // 1 ~ 3.DXILLib
    CreateShaderSubobject(kRayGenerationShader, kRayGenerationName);
    CreateShaderSubobject(kClosestHitShader, kRecursiveClosestHitName);
    CreateShaderSubobject(kMissShader, kRecursiveMissName);

    // 4.ヒットグループ
    auto hitGroup = stateObjectDesc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(kRecursiveClosestHitName);
    hitGroup->SetHitGroupExport(kRecursiveHitGroupName);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    // 5.ヒットグループのローカルルートシグネチャ
    auto hitGroupRootSignature = stateObjectDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
    hitGroupRootSignature->SetRootSignature(hitGroupLocalRootSignature_);

    // 6.ヒットグループアソシエーション
    auto hitGroupRootSignatureAssociation = stateObjectDesc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
    hitGroupRootSignatureAssociation->SetSubobjectToAssociate(*hitGroupRootSignature);
    hitGroupRootSignatureAssociation->AddExport(kRecursiveHitGroupName);

    // 7.ミスのローカルルートシグネチャ
    auto missRootSignature = stateObjectDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
    missRootSignature->SetRootSignature(missLocalRootSignature_);

    // 8.ミスアソシエーション
    auto missRootSignatureAssociation = stateObjectDesc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
    missRootSignatureAssociation->SetSubobjectToAssociate(*missRootSignature);
    missRootSignatureAssociation->AddExport(kRecursiveMissName);

    // 9.シェーダーコンフィグ
    auto shaderConfig = stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    size_t maxPayloadSize = 3 * sizeof(float) + 2 * sizeof(uint32_t);      // 最大ペイロードサイズ
    size_t maxAttributeSize = 2 * sizeof(float);   // 最大アトリビュートサイズ
    shaderConfig->Config((UINT)maxPayloadSize, (UINT)maxAttributeSize);

    // 10.パイプラインコンフィグ
    auto pipelineConfig = stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    uint32_t maxTraceRecursionDepth = 8; // 1 + 再帰回数
    pipelineConfig->Config(maxTraceRecursionDepth);

    // 11.グローバルルートシグネチャ
    auto globalRootSignature = stateObjectDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(globalRootSignature_);

    stateObject_.Create(L"RaytracingStateObject", stateObjectDesc);
}

void TestRTRenderer::CreateShaderTables() {
    {
        Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
        stateObject_.Get().As(&stateObjectProperties);

        auto InsertIdentifier = [&](const wchar_t* name) {
            identifierMap_[name] = stateObjectProperties->GetShaderIdentifier(name);
        };
        InsertIdentifier(kRayGenerationName);
        InsertIdentifier(kRecursiveHitGroupName);
        InsertIdentifier(kRecursiveMissName);
    }

    {
        ShaderRecord rayGenerationShaderRecord(identifierMap_[kRayGenerationName]);
        rayGenerationShaderTable_.Create(L"RayGenerationShaderTable", &rayGenerationShaderRecord, 1);
    }
}

void TestRTRenderer::BuildScene(CommandContext& commandContext, const ModelSorter& modelSorter) {

    struct MaterialData {
        Vector3 albedo;
        float metallic;
        Vector3 emissive;
        float roughness;
        uint32_t albedoMapIndex;
        uint32_t metallicRoughnessMapIndex;
        uint32_t normalMapIndex;
        uint32_t pad;
    };

    struct MeshProperty {
        MaterialData material;
        uint32_t vertexBufferIndex;
        uint32_t vertexOffset;
        uint32_t indexBufferIndex;
        uint32_t indexOffset;
    };

    uint32_t defaultWhiteTextureIndex = DefaultTexture::White.GetSRV().GetIndex();
    uint32_t defaultNormalTextureIndex = DefaultTexture::Normal.GetSRV().GetIndex();


    auto ErrorMaterial = [defaultWhiteTextureIndex, defaultNormalTextureIndex]() {
        MaterialData materialData;
        materialData.albedo = { 0.988f, 0.059f, 0.753f };
        materialData.metallic = 0.0f;
        materialData.emissive = { 0.0f, 0.0f, 0.0f };
        materialData.roughness = 0.0f;
        materialData.albedoMapIndex = defaultWhiteTextureIndex;
        materialData.metallicRoughnessMapIndex = defaultWhiteTextureIndex;
        materialData.normalMapIndex = defaultNormalTextureIndex;
        return materialData;
    };

    auto SetMaterialData = [](MaterialData& dest, const Material& src) {
        dest.albedo = src.albedo;
        dest.metallic = src.metallic;
        dest.emissive = src.emissive * src.emissiveIntensity;
        dest.roughness = src.roughness;
        if (src.albedoMap) { dest.albedoMapIndex = src.albedoMap->GetSRV().GetIndex(); }
        if (src.metallicRoughnessMap) { dest.metallicRoughnessMapIndex = src.metallicRoughnessMap->GetSRV().GetIndex(); }
        if (src.normalMap) { dest.normalMapIndex = src.normalMap->GetSRV().GetIndex(); }
    };


    auto& drawModels = modelSorter.GetDrawModels();

    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs;
    instanceDescs.reserve(drawModels.size());

    size_t numMeshes = 0;
    for (auto& instance : drawModels) {
        numMeshes += instance->GetModel()->GetMeshes().size();
    }

    size_t bufferSize = numMeshes * sizeof(MeshProperty);

    auto intermediateBuffer = commandContext.AllocateDynamicBuffer(LinearAllocatorType::Upload, bufferSize);
    std::span<MeshProperty> cpuBuffer = { reinterpret_cast<MeshProperty*>(intermediateBuffer.cpu), numMeshes };

    uint32_t copyLocation = 0;
    // レイトレで使用するオブジェクトをインスタンスデスクに登録
    for (auto& instance : drawModels) {

        auto model = instance->GetModel();

        auto& desc = instanceDescs.emplace_back();

        for (uint32_t y = 0; y < 3; ++y) {
            for (uint32_t x = 0; x < 4; ++x) {
                desc.Transform[y][x] = instance->GetWorldMatrix().m[x][y];
            }
        }
        desc.InstanceID = copyLocation;
        desc.InstanceMask = VISIBILITY_MASK;
        if (instance->BeReflected()) {
            desc.InstanceMask |= RECURSIVE_MASK;
        }
        desc.InstanceContributionToHitGroupIndex = 0;
        desc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

        const SkinCluster* skinningData = nullptr;
        if (auto skeleton = instance->GetSkeleton()) {
            auto skinCluster = RenderManager::GetInstance()->GetSkinningManager().GetSkinCluster(skeleton.get());
            if (skinCluster) {
                skinningData = skinCluster;
            }
        }
        desc.AccelerationStructure = skinningData == nullptr ? model->GetBLAS().GetGPUVirtualAddress() : skinningData->GetSkinnedBLAS().GetGPUVirtualAddress();

        auto instanceMaterial = instance->GetMaterial();

        for (auto& mesh : model->GetMeshes()) {
            auto& dest = cpuBuffer[copyLocation++];

            dest.vertexBufferIndex = model->GetVertexBuffer().GetSRV().GetIndex();
            dest.vertexOffset = mesh.vertexOffset;
            if (skinningData) {
                dest.vertexBufferIndex = skinningData->GetSkinnedVertexBuffer().GetSRV().GetIndex();
            }
            dest.indexBufferIndex = model->GetIndexBuffer().GetSRV().GetIndex();
            dest.indexOffset = mesh.indexOffset;

            MaterialData& materialData = dest.material;
            materialData = ErrorMaterial();
            // インスタンスのマテリアルを優先
            if (instanceMaterial) {
                SetMaterialData(materialData, *instanceMaterial);
            }
            // メッシュのマテリアル
            else if (mesh.material < model->GetMaterials().size()) {
                SetMaterialData(materialData, model->GetMaterials()[mesh.material]);
            }
        }
    }

    auto meshPropertiesBuffer = commandContext.AllocateDynamicBuffer(LinearAllocatorType::Default, bufferSize);
    commandContext.CopyBufferRegion(meshPropertiesBuffer.resource, meshPropertiesBuffer.offset, intermediateBuffer.resource, intermediateBuffer.offset, bufferSize);
    tlas_.Create(L"RaytracingRenderer TLAS", commandContext, instanceDescs.data(), instanceDescs.size());

    {
        ShaderRecord shaderRecord(identifierMap_[kRecursiveHitGroupName]);
        shaderRecord.Add(meshPropertiesBuffer.gpu);
        hitGroupShaderTable_.Create(L"RaytracingRenderer HitGroupShaderTable", &shaderRecord, 1);
    }

    struct SkyParameter {
        Vector3 sunPosition;
        float sunIntensity;

        float Kr;
        float Km;
        float innerRadius;
        float outerRadius;

        Vector3 invWaveLength;
        float scale;

        float scaleDepth;
        float scaleOverScaleDepth;
        float g;
        float exposure;
    } skyParameter;
    
    skyParameter.sunPosition = RenderManager::GetInstance()->GetSky().GetSunDirection();
    skyParameter.sunIntensity = 1300.0f;
    skyParameter.Kr = 0.0025f;
    skyParameter.Km = 0.0010f;
    skyParameter.innerRadius = 6.36e6f;
    skyParameter.outerRadius = 6.46e6f;
    Vector3 waveLength = { 0.680f, 0.550f, 0.440f };
    skyParameter.invWaveLength.x = 1.0f / std::pow(waveLength.x, 4.0f);
    skyParameter.invWaveLength.y = 1.0f / std::pow(waveLength.y, 4.0f);
    skyParameter.invWaveLength.z = 1.0f / std::pow(waveLength.z, 4.0f);
    skyParameter.scale = 1.0f / (skyParameter.outerRadius - skyParameter.innerRadius);
    skyParameter.scaleDepth = 0.25f;
    skyParameter.scaleOverScaleDepth = skyParameter.scale / skyParameter.scaleDepth;
    skyParameter.g = -0.999f;
    skyParameter.exposure = 0.05f;

    auto skyParameterAllocation = commandContext.TransfarUploadBuffer(sizeof(skyParameter), &skyParameter);

    // skyboxが変更された可能性あり
    {
        ShaderRecord missShaderRecord(identifierMap_[kRecursiveMissName]);
        missShaderRecord.Add(skyParameterAllocation);
        // skybox1
        D3D12_GPU_DESCRIPTOR_HANDLE skyboxSRV = DefaultTexture::BlackCubeMap.GetSRV();
        if (skyboxTexture_) {
            skyboxSRV = skyboxTexture_->GetSRV();
        }
        missShaderRecord.Add(skyboxSRV);
        // skybox1
        skyboxSRV = DefaultTexture::BlackCubeMap.GetSRV();
        if (skyboxRadianceTexture_) {
            skyboxSRV = skyboxRadianceTexture_->GetSRV();
        }
        missShaderRecord.Add(skyboxSRV);
        missShaderTable_.Create(L"MissShaderTable", &missShaderRecord, 1);
    }
}
