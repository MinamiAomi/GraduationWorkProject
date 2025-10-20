#pragma once

#include <vector>
#include <array>

#include "Core/UploadBuffer.h"
#include "Core/GPUBuffer.h"
#include "Math/MathUtils.h"
#include "Model.h"
#include "Skeleton.h"
#include "Raytracing/BLAS.h"

class SkinCluster {
    friend class SkinningManager;
public:
    static const uint32_t kNumMaxInfluence = 4;
    struct VertexInfluence {
        std::array<int32_t, kNumMaxInfluence> jointIndices;
        std::array<float, kNumMaxInfluence> weights;
    };

    struct Well {
        Matrix4x4 skeletonSpaceMatrix;
        Matrix4x4 skeletonSpaceInverseTransposeMatrix;
    };

    void Create(CommandContext& commandContext, const std::shared_ptr<Model>& model, const Skeleton& skeleton);
    void Update(CommandContext& commandContext, const Skeleton& skeleton);

    uint32_t GetNumVertices() const { return numVertices_; }
    const StructuredBuffer& GetSkinnedVertexBuffer() const { return skinnedVertexBuffer_; }
    const BLAS& GetSkinnedBLAS() const { return skinnedBLAS_; }


private:
    std::shared_ptr<Model> model_;
    std::vector<Matrix4x4> inverseBindPoseMatrices_;
    StructuredBuffer vertexInfluenceBuffer_;
    StructuredBuffer matrixPaletteBuffer_;
    StructuredBuffer skinnedVertexBuffer_;
    BLAS skinnedBLAS_;
    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> blasDescs_;
    uint32_t numVertices_;
};