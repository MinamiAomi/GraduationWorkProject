#pragma once

#include <filesystem>
#include <memory>
#include <list>
#include <vector>

#include "Math/MathUtils.h"
#include "Core/GPUBuffer.h"
//#include "Mesh.h"
#include "Node.h"
#include "Raytracing/BLAS.h"
#include "Skeleton.h"

class Model {
public:
    struct Vertex {
        Vector3 position;
        uint32_t normal;
        uint32_t tangent;
        Vector2 texcood;
    };

    using Index = uint32_t;

    struct VertexWeightData {
        float weight;
        uint32_t vertexIndex;
    };

    struct JointWeightData {
        Matrix4x4 inverseBindPoseMatrix;
        std::vector<VertexWeightData> vertexWeights;
    };

    struct Mesh {
        uint32_t vertexOffset;
        uint32_t vertexCount;
        uint32_t indexOffset;
        uint32_t indexCount;
        uint32_t material;
    };

    static std::shared_ptr<Model> Load(const std::filesystem::path& path);

    const BLAS& GetBLAS() const { return blas_; }
    const std::vector<Mesh>& GetMeshes() const { return meshes_; }
    const StructuredBuffer& GetVertexBuffer() const { return vertexBuffer_; }
    const StructuredBuffer& GetIndexBuffer() const { return indexBuffer_; }
    const std::vector<Vertex>& GetVertices() const { return vertices_; }
    const std::vector<Index>& GetIndices() const { return indices_; }
    const std::vector<Material>& GetMaterials() const { return materials_; }
    const std::map<std::string, JointWeightData> GetSkinClusterData() const { return skinClusterData_; }
    const Node& GetRootNode() const { return rootNode_; }
    size_t GetNumVertices() const { return vertices_.size(); }
    size_t GetNumIndices() const { return indices_.size(); }

private:
    Model() = default;
    ~Model() = default;

    std::vector<Mesh> meshes_;
    std::vector<Vertex> vertices_;
    std::vector<Index> indices_;
    std::vector<Material> materials_;
    std::map<std::string, JointWeightData> skinClusterData_;
    
    StructuredBuffer vertexBuffer_;
    StructuredBuffer indexBuffer_;

    BLAS blas_;
    Node rootNode_;
};

class ModelInstance {
public:
    static const std::list<ModelInstance*>& GetInstanceList() { return instanceLists_; }

    ModelInstance();
    ~ModelInstance();

    void SetModel(const std::shared_ptr<Model>& model) { model_ = model; }
    void SetSkeleton(const std::shared_ptr<Skeleton> skeleton) { skeleton_ = skeleton; }
    void SetMaterial(const std::shared_ptr<Material> material) { material_ = material; }
    void SetWorldMatrix(const Matrix4x4& worldMatrix) { worldMatrix_ = worldMatrix; }
    void SetColor(const Vector3& color) { color_ = color; }
    void SetAlpha(float alpha) { alpha_ = alpha; }
    void SetBeReflected(bool beReflected) { beReflected_ = beReflected; }
    void SetRefraction(bool refrection) { refraction_ = refrection; }
    void SetReciveShadow(bool reciveShadow) { reciveShadow_ = reciveShadow; }
    void SetCastShadow(bool castShadow) { castShadow_ = castShadow; }
    void SetReflection(bool reflection) { reflection_ = reflection; }
    void SetUseLighting(bool useLighting) { useLighting_ = useLighting; }
    void SetIsActive(bool isActive) { isActive_ = isActive; }


    const std::shared_ptr<Model>& GetModel() const { return model_; }
    const std::shared_ptr<Skeleton>& GetSkeleton() const { return skeleton_; }
    const std::shared_ptr<Material>& GetMaterial() const { return material_; }
    const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
    const Vector3& GetColor() const { return color_; }
    float GetAlpha() const { return alpha_; }
    bool BeReflected() const { return beReflected_; }
    bool Refraction() const { return refraction_; }
    bool ReciveShadow() const { return reciveShadow_; }
    bool CastShadow() const { return castShadow_; }
    bool Reflection() const { return reflection_; }
    bool UseLighting() const { return useLighting_; }
    bool IsActive() const { return isActive_; }

    void SetAlphaTest(bool alphaTest) { alphaTest_ = alphaTest; }
    bool AlphaTest() const { return alphaTest_; }

private:
    static std::list<ModelInstance*> instanceLists_;

    std::shared_ptr<Model> model_;
    std::shared_ptr<Skeleton> skeleton_;
    std::shared_ptr<Material> material_;
    Matrix4x4 worldMatrix_;
    Vector3 color_ = Vector3::one;
    float alpha_ = 1.0f;
    
    // 映り込みフラグ
    bool beReflected_ = true;
    // ガラス面
    bool refraction_ = false;
    bool reciveShadow_ = true;
    bool castShadow_ = true;
    bool reflection_ = false;
    bool useLighting_ = true;
    bool isActive_ = true;

    // 本来マテリアルにあるべき
    bool alphaTest_ = false;
};

