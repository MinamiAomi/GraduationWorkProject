#pragma once

#include <map>
#include <memory>

#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Core/ColorBuffer.h"
#include "Core/UploadBuffer.h"

class CommandContext;

class GaussianBlur {
public:
    GaussianBlur();
    ~GaussianBlur();

    void Initialize(ColorBuffer* originalTexture);
    void Render(CommandContext& commandContext);
    void UpdateWeightTable(float blurPower);

    ColorBuffer& GetResult() { return verticalBlurTexture_; }

private:
    static const uint32_t kNumWeights = 8;

    GaussianBlur(const GaussianBlur&) = delete;
    GaussianBlur& operator=(const GaussianBlur&) = delete;


    ColorBuffer* originalTexture_ = nullptr;
    ColorBuffer horizontalBlurTexture_;
    ColorBuffer verticalBlurTexture_;
    UploadBuffer constantBuffer_;
    float weights_[kNumWeights]{};
};