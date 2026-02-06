#pragma once

#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Math/MathUtils.h"
#include "Math/Camera.h"

class CommandContext;
class ColorBuffer;
class DepthBuffer;

class FogPostEffect {
public:
    void Initialize();
    void Dispatch(CommandContext& commandContext, ColorBuffer& texture, DepthBuffer& depth, const Camera& camera);

    void SetColor(const Vector3& color) { color_ = color; }
    const Vector3& GetColor() const { return color_; }

    void SetDepthFogStart(float depthFogStart) { depthFogStart_ = depthFogStart; }
    void SetDepthFogEnd(float depthFogEnd) { depthFogEnd_ = depthFogEnd; }
    void SetHightFogStart(float hightFogStart) { hightFogStart_ = hightFogStart; }
    void SetHightFogEnd(float hightFogEnd) { hightFogStart_ = hightFogEnd; }

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
    Vector3 color_ = Vector3::one;
    float depthFogStart_ = 200.0f;
    float depthFogEnd_ = 500.0f;
    float hightFogStart_ = 0.0f;
    float hightFogEnd_ = -10.0f;
};