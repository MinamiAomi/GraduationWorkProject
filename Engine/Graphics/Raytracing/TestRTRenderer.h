#pragma once

#include <map>
#include <string>

#include "../Core/ColorBuffer.h"
#include "../Core/RootSignature.h"
#include "../../Math/Camera.h"
#include "../Core/TextureResource.h"

#include "StateObject.h"
#include "TLAS.h"
#include "ShaderTable.h"
#include "SpatialDenoiser.h"
#include "TemporalDenoiser.h"

class CommandContext;
class ModelSorter;

class TestRTRenderer {
public:
    void Create(uint32_t width, uint32_t height);

    void Render(CommandContext& commandContext, const Camera& camera, const ModelSorter& modelSorter);

    void SetSkybox(const std::shared_ptr<TextureResource>& texture) { skyboxTexture_ = texture; }
    void SetSkyboxRadiance(const std::shared_ptr<TextureResource>& texture) { skyboxRadianceTexture_ = texture; }
    ColorBuffer& GetResult() { return denoisedBuffer_; }
    ColorBuffer& GetIntermadiate() { return intermediateBuffer_; }

private:
    void CreateRootSignature();
    void CreateStateObject();
    void CreateShaderTables();
    void BuildScene(CommandContext& commandContext, const ModelSorter& modelSorter);

    StateObject stateObject_;
    RootSignature globalRootSignature_;
    RootSignature hitGroupLocalRootSignature_;
    RootSignature missLocalRootSignature_;

    TLAS tlas_;

    ShaderTable rayGenerationShaderTable_;
    ShaderTable hitGroupShaderTable_;
    ShaderTable missShaderTable_;

    std::map<std::wstring, void*> identifierMap_;
    std::shared_ptr<TextureResource> skyboxTexture_;
    std::shared_ptr<TextureResource> skyboxRadianceTexture_;
    ColorBuffer intermediateBuffer_;
    ColorBuffer denoisedBuffer_;
    int32_t time_;

    SpatialDenoiser spatialDenoiser_;
    TemporalDenoiser temporalDenoiser_;
};