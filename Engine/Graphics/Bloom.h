#pragma once

#include "GaussianBlur.h"

class CommandContext;

class Bloom {
public:
    static const uint32_t kMaxLevel = 4;
    
    void Initialize(ColorBuffer* originalTexture);
    void Render(CommandContext& commandContext, uint32_t level = kMaxLevel);
    
    void SetThreshold(float threshold) { threshold_ = threshold; }
    void SetKnee(float knee) { knee_ = knee; }

    ColorBuffer& GetResult() { return *originalTexture_; }
    
    ColorBuffer& GetLuminanceTexture() { return luminanceTexture_; }

private:
    RootSignature rootSignature_;
    PipelineState luminacePipelineState_;
    PipelineState additivePipelineState_;

    ColorBuffer* originalTexture_;
    ColorBuffer luminanceTexture_;
    GaussianBlur gaussianBlurs_[kMaxLevel];

    float threshold_ = 0.85f;
    float knee_ = 0.3f;
    float intensity_ = 1.0f;
};