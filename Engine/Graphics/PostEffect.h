#pragma once

#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Core/ColorBuffer.h"
#include "Math/MathUtils.h"

class CommandContext;

class PostEffect {
public:
    struct Grayscale {
        Vector3 color = Vector3::one;
        bool isActive = false;
    };
    struct Vignette {
        float intensity = 16.0f;
        float power = 0.8f;
        bool isActive = false;
    };

    struct HSVFilter {
        Vector3 bias = { 0.0f, 0.0f, 0.0f };
        Vector3 factor = { 1.0f, 1.0f, 1.0f };
    };

    void Initialize(const ColorBuffer& target);
    void Render(CommandContext& commandContext, ColorBuffer& texture);

    void DrawImGui(const char* label);

    Grayscale& GetGrayscale() { return grayscale_; }
    Vignette& GetVignette() { return vignette_; }
    HSVFilter& GetHSVFilter() { return hsvFilter_; }

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
    Grayscale grayscale_;
    Vignette vignette_;
    HSVFilter hsvFilter_;
};