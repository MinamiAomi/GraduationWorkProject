#include "ParticleCore.h"

#include "Core/Helper.h"
#include "Core/ShaderManager.h"
#include "Core/CommandContext.h"
#include "Math/Camera.h"
#include "Shader/Particle/Particle.h"

namespace {

    static const wchar_t kInitializeCS[] = L"Particle/InitializeParticleCS.hlsl";
    static const wchar_t kEmitCS[] = L"Particle/EmitParticleCS.hlsl";
    static const wchar_t kUpdateCS[] = L"Particle/UpdateParticleCS.hlsl";
    static const wchar_t kDrawVS[] = L"Particle/DrawParticleVS.hlsl";
    static const wchar_t kDrawPS[] = L"Particle/DrawParticlePS.hlsl";
}

void ParticleCore::Initialize(DXGI_FORMAT rtvFormat) {

    RootSignatureDescHelper rsDesc;
    //　コンピュート
    rsDesc.AddDescriptorTable().AddUAVDescriptors(1, 0, 1);
    rsDesc.AddDescriptorTable().AddUAVDescriptors(1, 1, 1);
    rsDesc.AddDescriptorTable().AddUAVDescriptors(1, 2, 1);
    rsDesc.AddConstantBufferView(0, 1);
    rsDesc.AddConstantBufferView(0);
    rootSignatures_[RootSignatureType::ComputeRS].Create(L"ParticleCore ComputeRootSignature", rsDesc);
    rsDesc.Clear();
    // グラフィックス
    rsDesc.AddConstantBufferView(0);
    rsDesc.AddDescriptorTable().AddSRVDescriptors(1, 0);
    rsDesc.AddFlag(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    rootSignatures_[RootSignatureType::GraphicsRS].Create(L"ParticleCore GraphicsRootSignature", rsDesc);

    auto shaderManager = ShaderManager::GetInstance();
    auto initializeCS = shaderManager->Compile(kInitializeCS, ShaderType::Compute, 6, 6);
    auto emitCS = shaderManager->Compile(kEmitCS, ShaderType::Compute, 6, 6);
    auto updateCS = shaderManager->Compile(kUpdateCS, ShaderType::Compute, 6, 6);
    auto drawVS = shaderManager->Compile(kDrawVS, ShaderType::Vertex, 6, 6);
    auto drawPS = shaderManager->Compile(kDrawPS, ShaderType::Pixel, 6, 6);

    D3D12_COMPUTE_PIPELINE_STATE_DESC cpsoDesc{};
    cpsoDesc.pRootSignature = rootSignatures_[RootSignatureType::ComputeRS];
    cpsoDesc.CS = CD3DX12_SHADER_BYTECODE(initializeCS->GetBufferPointer(), initializeCS->GetBufferSize());
    pipelineStates_[PipelineStateType::InitializeCPSO].Create(L"ParticleCore InitializeCPSO", cpsoDesc);
    cpsoDesc.CS = CD3DX12_SHADER_BYTECODE(emitCS->GetBufferPointer(), emitCS->GetBufferSize());
    pipelineStates_[PipelineStateType::EmitCPSO].Create(L"ParticleCore EmitCPSO", cpsoDesc);
    cpsoDesc.CS = CD3DX12_SHADER_BYTECODE(updateCS->GetBufferPointer(), updateCS->GetBufferSize());
    pipelineStates_[PipelineStateType::UpdateCPSO].Create(L"ParticleCore UpdateCPSO", cpsoDesc);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsoDesc{};
    gpsoDesc.pRootSignature = rootSignatures_[RootSignatureType::GraphicsRS];
    gpsoDesc.VS = CD3DX12_SHADER_BYTECODE(drawVS->GetBufferPointer(), drawVS->GetBufferSize());
    gpsoDesc.PS = CD3DX12_SHADER_BYTECODE(drawPS->GetBufferPointer(), drawPS->GetBufferSize());

    D3D12_INPUT_ELEMENT_DESC inputElements[] = {
         { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    D3D12_INPUT_LAYOUT_DESC inputLayout{};
    inputLayout.NumElements = _countof(inputElements);
    inputLayout.pInputElementDescs = inputElements;
    gpsoDesc.InputLayout = inputLayout;

    gpsoDesc.BlendState = Helper::BlendAdditive;
    gpsoDesc.RasterizerState = Helper::RasterizerNoCull;
    gpsoDesc.NumRenderTargets = 1;
    gpsoDesc.RTVFormats[0] = rtvFormat;
    gpsoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    gpsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    gpsoDesc.SampleDesc.Count = 1;
    pipelineStates_[PipelineStateType::DrawGPSO].Create(L"ParticleCore DrawGPSO", gpsoDesc);

    particleBuffer_.Create(L"ParticleCore ParticlesBuffer", MAX_PARTICLES, sizeof(GPUParticleStructs::Particle));
    freeListIndexBuffer_.Create(L"ParticleCore FreeListIndex", 1, sizeof(int32_t));
    freeListBuffer_.Create(L"ParticleCore FreeList", MAX_PARTICLES, sizeof(uint32_t));

    time_ = 0.0f;

    CommandContext commandContext;
    commandContext.Start(D3D12_COMMAND_LIST_TYPE_DIRECT);
    commandContext.TransitionResource(freeListIndexBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.TransitionResource(freeListBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Reset(commandContext);
    commandContext.Finish(true);
}

void ParticleCore::Dispatch(CommandContext& commandContext) {
    time_ += 0.016f;

    GPUParticleStructs::PerFrame perFrame{};
    perFrame.deltaTime = 0.016f;
    perFrame.time = time_ += perFrame.deltaTime;

    GPUParticleStructs::EmitterParameter emitter;
    emitter.initialSpeed = 0.1f;
    emitter.maxLifeTime = 3.0f;
    emitter.minLifeTime = 2.0f;
    emitter.translate = { 0.0f, 0.0f, 0.0f };
    emitter.radius = 10.0f;
    emitter.minScale = { 0.2f, 0.2f, 0.2f };
    emitter.maxScale = { 1.0f, 1.0f, 1.0f };
    emitter.count = 64;

    static uint32_t count = 0;
    count += 1;

    commandContext.BeginEvent(L"Particle Dipatch");
    // Emit
    commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.SetComputeRootSignature(rootSignatures_[RootSignatureType::ComputeRS]);
    commandContext.SetComputeDescriptorTable(0, particleBuffer_.GetUAV());
    commandContext.SetComputeDescriptorTable(1, freeListIndexBuffer_.GetUAV());
    commandContext.SetComputeDescriptorTable(2, freeListBuffer_.GetUAV());
    commandContext.SetComputeDynamicConstantBufferView(3, sizeof(perFrame), &perFrame);
    commandContext.SetComputeDynamicConstantBufferView(4, sizeof(emitter), &emitter);
    if ((count % 10) == 0) {
        commandContext.SetPipelineState(pipelineStates_[PipelineStateType::EmitCPSO]);
        commandContext.Dispatch(uint32_t(emitter.count + NUM_THREADS - 1) / NUM_THREADS);
        commandContext.UAVBarrier(particleBuffer_);
        commandContext.UAVBarrier(freeListIndexBuffer_);
        commandContext.UAVBarrier(freeListBuffer_);
    }
    // Update
    commandContext.SetPipelineState(pipelineStates_[PipelineStateType::UpdateCPSO]);
    commandContext.Dispatch(uint32_t(MAX_PARTICLES + NUM_THREADS - 1) / NUM_THREADS);
    commandContext.UAVBarrier(particleBuffer_);
    // Draw
    commandContext.EndEvent();
}

void ParticleCore::Render(CommandContext& commandContext, const Camera& camera) {
    struct Vertex {
        Vector3 position;
        Vector2 texcoord;
    };
    Vertex vertices[] = {
        {{-1.0f,-1.0f, 0.0f }, { 0.0f, 1.0f }},
        {{-1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }},
        {{ 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
        {{-1.0f,-1.0f, 0.0f }, { 0.0f, 1.0f }},
        {{ 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
        {{ 1.0f,-1.0f, 0.0f }, { 1.0f, 1.0f }},
    };

    struct PreView {
        Matrix4x4 viewProjectionMatrix;
        Matrix4x4 billboardMatrix;
    }preView;
    preView.viewProjectionMatrix = camera.GetViewProjectionMatrix();
    preView.billboardMatrix = Matrix4x4::MakeRotation(camera.GetRotate());

    commandContext.BeginEvent(L"Particle Render");

    commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    commandContext.SetRootSignature(rootSignatures_[RootSignatureType::GraphicsRS]);
    commandContext.SetDynamicConstantBufferView(0, sizeof(preView), &preView);
    commandContext.SetDescriptorTable(1, particleBuffer_.GetSRV());
    commandContext.SetPipelineState(pipelineStates_[PipelineStateType::DrawGPSO]);
    commandContext.SetDynamicVertexBuffer(0, _countof(vertices), sizeof(vertices[0]), vertices);
    commandContext.DrawInstanced(_countof(vertices), MAX_PARTICLES);

    commandContext.EndEvent();
}

void ParticleCore::Reset(CommandContext& commandContext) {

    commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandContext.SetComputeRootSignature(rootSignatures_[RootSignatureType::ComputeRS]);
    commandContext.SetComputeDescriptorTable(0, particleBuffer_.GetUAV());
    commandContext.SetComputeDescriptorTable(1, freeListIndexBuffer_.GetUAV());
    commandContext.SetComputeDescriptorTable(2, freeListBuffer_.GetUAV());
    commandContext.SetPipelineState(pipelineStates_[PipelineStateType::InitializeCPSO]);
    commandContext.Dispatch(uint32_t(MAX_PARTICLES + NUM_THREADS - 1) / NUM_THREADS);
    commandContext.UAVBarrier(particleBuffer_);
    commandContext.UAVBarrier(freeListIndexBuffer_);
    commandContext.UAVBarrier(freeListBuffer_);
    commandContext.FlushResourceBarriers();
}
