#pragma once

#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Math/MathUtils.h"

class CommandContext;
class ColorBuffer;

class Grayscale {
public:
    void Initialize();
    void Dispatch(CommandContext& commandContext, ColorBuffer& texture);

    void SetColor(const Vector3& color) { color_ = color; }
    const Vector3& GetColor() const { return color_; }

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
    Vector3 color_ = Vector3::one;
};