#pragma once
#pragma once

#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Math/MathUtils.h"

class CommandContext;
class ColorBuffer;

class FinalEffect {
public:
    void Initialize();
    void Dispatch(CommandContext& commandContext, ColorBuffer& texture);

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
  
};