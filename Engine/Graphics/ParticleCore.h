#pragma once

#include "Math/MathUtils.h"
#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Core/GPUBuffer.h"

class CommandContext;
class Camera;

class ParticleCore {
public:
    struct RootSignatureType {
        enum Enum {
            ComputeRS,
            GraphicsRS,

            NumRootSignatureTypes
        };
    };

    struct PipelineStateType {
        enum Enum {
            InitializeCPSO,
            EmitCPSO,
            UpdateCPSO,
            DrawGPSO,

            NumPipelineTypes
        };
    };

    void Initialize(DXGI_FORMAT rtvFormat);
    void Dispatch(CommandContext& commandContext);
    void Render(CommandContext& commandContext, const Camera& camera);

    void Reset(CommandContext& commandContext);

private:
    RootSignature rootSignatures_[RootSignatureType::NumRootSignatureTypes];
    PipelineState pipelineStates_[PipelineStateType::NumPipelineTypes];
    StructuredBuffer particleBuffer_;
    StructuredBuffer freeListIndexBuffer_;
    StructuredBuffer freeListBuffer_;
    float time_;
};