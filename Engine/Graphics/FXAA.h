#pragma once
#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Core/ColorBuffer.h"

class CommandContext;

class FXAA {
public:
    void Initialize(ColorBuffer* original);
    void Render(CommandContext& commandContext);
    ColorBuffer& GetResult() { return result_; }

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
    ColorBuffer result_;
    ColorBuffer* original_;
};