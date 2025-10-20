#include "RaymarchingRenderer.h"

#include "Core/CommandContext.h"
#include "Core/ShaderManager.h"

const wchar_t kRaymarchingShader[] = L"Raymarching.hlsl";

void RaymarchingRenderer::Create(uint32_t width, uint32_t height) {

    CD3DX12_DESCRIPTOR_RANGE ranges[1]{};
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[2]{};
    rootParameters[0].InitAsConstantBufferView(0);
    rootParameters[1].InitAsDescriptorTable(_countof(ranges), ranges);

    D3D12_ROOT_SIGNATURE_DESC rsDesc{};
    rsDesc.pParameters = rootParameters;
    rsDesc.NumParameters = _countof(rootParameters);
    rootSignature_.Create(L"RaymarchingRenderer RootSignature", rsDesc);

    D3D12_COMPUTE_PIPELINE_STATE_DESC psDesc{};
    psDesc.pRootSignature = rootSignature_;
    auto cs = ShaderManager::GetInstance()->Compile(kRaymarchingShader, ShaderType::Compute, 6, 6);
    psDesc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
    pipelineState_.Create(L"RaymarchingRenderer PipelineState", psDesc);

    resultBuffer_.Create(L"RaymarchingRenderer ResultBuffer", width, height, DXGI_FORMAT_R8G8B8A8_UNORM);

    time_ = 0.0f;
}

void RaymarchingRenderer::Render(CommandContext& commandContext, const Camera& camera) {
    struct Scene {
        Matrix4x4 viewProjectionInverse;
        Vector3 cameraPosition;
        float time;
    };

    time_ += 1.0f / kCycle;
    time_ = time_ - std::floor(time_);

    Scene scene;
    scene.viewProjectionInverse = camera.GetViewProjectionMatrix().Inverse();
    scene.cameraPosition = camera.GetPosition();
    scene.time = time_;

    commandContext.TransitionResource(resultBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    commandContext.SetComputeRootSignature(rootSignature_);
    commandContext.SetPipelineState(pipelineState_);
    commandContext.SetComputeDynamicConstantBufferView(0, sizeof(scene), &scene);
    commandContext.SetComputeDescriptorTable(1, resultBuffer_.GetUAV());

    const UINT kNumThreadsPerGroupX = 8;
    const UINT kNumThreadsPerGroupY = 8;
    commandContext.Dispatch((UINT)std::ceil(resultBuffer_.GetWidth() / kNumThreadsPerGroupX), (UINT)std::ceil(resultBuffer_.GetHeight() / kNumThreadsPerGroupY));
    commandContext.UAVBarrier(resultBuffer_);
    commandContext.TransitionResource(resultBuffer_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandContext.FlushResourceBarriers();
}
