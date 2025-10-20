#pragma once

#include <map>
#include <string>

#include "../Core/ColorBuffer.h"
#include "../Core/RootSignature.h"

#include "StateObject.h"
#include "TLAS.h"
#include "ShaderTable.h"

class CommandContext;
class Camera;
class ModelSorter;

class Pathtracer {
public:
    void Initialize(uint32_t width, uint32_t height);

    void Dispatch(CommandContext& commandContext, const Camera& camera, const ModelSorter& modelSorter);

    ColorBuffer& GetResult() { return resultBuffer_; }

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
    ColorBuffer resultBuffer_;
    int32_t time_;
};