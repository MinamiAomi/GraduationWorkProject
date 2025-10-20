#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>

#include "Core/GPUBuffer.h"
#include "Math/MathUtils.h"
#include "Material.h"

class CommandContext;

struct Mesh {
    struct Vertex {
        Vector3 position;
        uint32_t normal;
        uint32_t tangent;
        Vector2 texcood;
    };

    struct VertexWeightData {
        float weight;
        uint32_t vertexIndex;
    };

    struct JointWeightData {
        Matrix4x4 inverseBindPoseMatrix;
        std::vector<VertexWeightData> vertexWeights;
    };

    using Index = uint32_t;

    StructuredBuffer vertexBuffer;
    StructuredBuffer indexBuffer;

    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    std::map<std::string, JointWeightData> skinClusterData;
    std::shared_ptr<Material> material;


    void CreateBuffers(CommandContext& commandContext);
};