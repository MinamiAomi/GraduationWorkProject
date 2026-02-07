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
    
    void SetDepthFogColor(const Vector3& depthFogColor) { depthFogColor_ = depthFogColor; }
    void SetHightFogColor(const Vector3& hightFogColor) { hightFogColor_ = hightFogColor; }
    void SetDepthFogStart(float depthFogStart) { depthFogStart_ = depthFogStart; }
    void SetDepthFogEnd(float depthFogEnd) { depthFogEnd_ = depthFogEnd; }
    void SetHightFogStart(float hightFogStart) { hightFogStart_ = hightFogStart; }
    void SetHightFogEnd(float hightFogEnd) { hightFogStart_ = hightFogEnd; }
    void SetFogFactor(float fogFactor) { fogFactor_ = fogFactor; }

    const Vector3& GetDepthFogColor() const { return depthFogColor_; }
    const Vector3& GetHightFogColor() const { return hightFogColor_; }
    float GetDepthFogStart() const { return depthFogStart_; }
    float GetDepthFogEnd() const { return depthFogEnd_; }
    float GetHightFogStart() const { return hightFogStart_; }
    float GetHightFogEnd() const { return hightFogEnd_; }
    float GetFogFactor() const { return fogFactor_; }

    void DrawImGui(const char* label);

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
    Vector3 depthFogColor_ = { 0.1f, 0.01f, 0.01f };
    Vector3 hightFogColor_ = { 0.1f, 0.01f, 0.01f };
    float depthFogStart_ = 50.0f;
    float depthFogEnd_ = 150.0f;
    float hightFogStart_ = 0.0f;
    float hightFogEnd_ = -5.0f;
    float fogFactor_ = 0.9f;
};