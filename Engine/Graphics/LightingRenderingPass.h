#pragma once
#include <memory>

#include "Core/ColorBuffer.h"
#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Core/TextureResource.h"
#include "Math/Camera.h"
#include "LightManager.h"

class CommandContext;
class GeometryRenderingPass;

class LightingRenderingPass {
public:
    struct RootIndex {
        enum Parameters {
            Scene,
            Sky,
            Albedo,
            MetallicRoughness,
            Normal,
            Depth,
            Irradiance,
            Radiance,

            NumRootParameters
        };
    };

    void Initialize(uint32_t width, uint32_t height);
    void Render(CommandContext& commandContext, GeometryRenderingPass& geometryRenderingPass, const Camera& camera, const DirectionalLight& light);

    void SetIrradianceTexture(const std::shared_ptr<TextureResource>& texture) { irradianceTexture_ = texture; }
    void SetRadianceTexture(const std::shared_ptr<TextureResource>& texture) { radianceTexture_ = texture; }
    ColorBuffer& GetResult() { return result_; }

private:
    ColorBuffer result_;
    RootSignature rootSignature_;
    PipelineState pipelineState_;
    std::shared_ptr<TextureResource> irradianceTexture_;
    std::shared_ptr<TextureResource> radianceTexture_;
};