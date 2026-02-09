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
            LightList,
            Albedo,
            MetallicRoughness,
            Normal,
            Emissive,
            Depth,

            NumRootParameters
        };
    };

    void Initialize(uint32_t width, uint32_t height);
    void Render(CommandContext& commandContext, GeometryRenderingPass& geometryRenderingPass, const Camera& camera, const DirectionalLight& light);
    void Render(CommandContext& commandContext, GeometryRenderingPass& geometryRenderingPass, const Camera& camera, const LightManager& lightManager);

    ColorBuffer& GetResult() { return result_; }

private:
    ColorBuffer result_;
    RootSignature rootSignature_;
    PipelineState pipelineState_;
};