#pragma once

#include "../Core/RootSignature.h"
#include "../Core/PipelineState.h"
#include "../Core/ColorBuffer.h"
#include "../GeometryRenderingPass.h"

class CommandContext;

class SpatialDenoiser {
public:
    struct Settings {
        float albedoSigma = 0.125f;
        float normalSigma = 0.125f;
        float depthSigma = 0.125f;
    };

    void Initialize(uint32_t width, uint32_t height, DXGI_FORMAT format);
    void Dispatch(CommandContext& commandContext, ColorBuffer& sourceBuffer, GeometryRenderingPass& gBuffers);

    ColorBuffer& GetDenoisedBuffer() { return denoisedBuffer_; }
    Settings& GetSettings() { return settings_; }
    const Settings& GetSettings() const { return settings_; }


private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
    ColorBuffer denoisedBuffer_;
    Settings settings_;

};
