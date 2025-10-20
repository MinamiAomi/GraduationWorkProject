#pragma once

#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Core/ColorBuffer.h"
#include "../Math/Camera.h"

class CommandContext;

class RaymarchingRenderer {
public:
    static const uint32_t kCycle = 300;
  
    void Create(uint32_t width, uint32_t height);

    void Render(CommandContext& commandContext, const Camera& camera);

    ColorBuffer& GetResult() { return resultBuffer_; }

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
    ColorBuffer resultBuffer_;
    float time_;
};