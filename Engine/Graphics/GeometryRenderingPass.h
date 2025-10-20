#pragma once
#include <memory>

#include "Core/ColorBuffer.h"
#include "Core/DepthBuffer.h"
#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Math/Camera.h"
#include "ModelSorter.h"

class CommandContext;

struct GBuffer {
    enum Type {
        Albedo,
        MetallicRoughness,
        Normal,
        ViewDepth,
        MeshMaterialIDs,

        NumGBuffers
    };
};

class GeometryRenderingPass {
public:
    struct RootIndex {
        enum Parameters {
            Scene,
            Instance,
            Material,
            BindlessTexture,

            NumRootParameters
        };
    };

    void Initialize(uint32_t width, uint32_t height);
    void Render(CommandContext& commandContext, const Camera& camera, const ModelSorter& modelSorter);

    ColorBuffer& GetGBuffer(GBuffer::Type type) { return gBuffers_[type]; }
    ColorBuffer& GetAlbedo() { return gBuffers_[GBuffer::Albedo]; }
    ColorBuffer& GetMetallicRoughness() { return gBuffers_[GBuffer::MetallicRoughness]; }
    ColorBuffer& GetNormal() { return gBuffers_[GBuffer::Normal]; }
    ColorBuffer& GetViewDepth() { return gBuffers_[GBuffer::ViewDepth]; }
    ColorBuffer& GetMeshMaterialIDs() { return gBuffers_[GBuffer::MeshMaterialIDs]; }
    DepthBuffer& GetDepth() { return depth_; }

private:
    ColorBuffer gBuffers_[GBuffer::NumGBuffers];
    DepthBuffer depth_;

    RootSignature rootSignature_;
    PipelineState pipelineState_;
};