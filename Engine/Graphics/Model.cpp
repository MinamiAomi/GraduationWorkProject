#include "Model.h"

#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Core/CommandContext.h"
#include "Core/TextureLoader.h"
#include "Material.h"

namespace {
    // Vector3からuint32_tに変換する
    uint32_t R32G32B32ToR10G10B10A2(const Vector3& in) {
        uint32_t x = static_cast<uint32_t>(std::clamp((in.x + 1.0f) * 0.5f, 0.0f, 1.0f) * 0x3FF) & 0x3FF;
        uint32_t y = static_cast<uint32_t>(std::clamp((in.y + 1.0f) * 0.5f, 0.0f, 1.0f) * 0x3FF) & 0x3FF;
        uint32_t z = static_cast<uint32_t>(std::clamp((in.z + 1.0f) * 0.5f, 0.0f, 1.0f) * 0x3FF) & 0x3FF;
        return x | y << 10 | z << 20;
    }

    Vector3 GenerateTangent(const Vector3& normal) {
        if (std::abs(Vector3::Dot(normal, Vector3::unitZ)) > 0.999f) {
            return Vector3::Cross(normal, Vector3::unitX);
        }
        return Vector3::Cross(normal, Vector3::unitZ);
    }

    // aiSceneからメッシュ配列を解析する
    std::vector<Model::Mesh> ParseMeshes(const aiScene* scene, const std::vector<Material>& materials, std::vector<Model::Vertex>& vertices, std::vector<Model::Index>& indices, std::map<std::string, Model::JointWeightData>& skinClusterData) {
        std::vector<Model::Mesh> meshes(scene->mNumMeshes);
        
        vertices.clear();
        indices.clear();

        for (uint32_t meshIndex = 0; auto & destMesh : meshes) {
            const aiMesh* srcMesh = scene->mMeshes[meshIndex];
            assert(srcMesh->HasNormals());

            destMesh.vertexOffset = (uint32_t)vertices.size();
            destMesh.vertexCount = (uint32_t)srcMesh->mNumVertices;
            for (uint32_t vertexIndex = 0; vertexIndex < srcMesh->mNumVertices; ++vertexIndex) {
                aiVector3D& srcPosition = srcMesh->mVertices[vertexIndex];
                aiVector3D& srcNormal = srcMesh->mNormals[vertexIndex];
                // セット
                Model::Vertex destVertex;
                destVertex.position = { srcPosition.x, srcPosition.y, srcPosition.z };
                Vector3 tmpNormal = { srcNormal.x, srcNormal.y, srcNormal.z };
                Vector3 tmpTangent;
                
                if (srcMesh->HasTangentsAndBitangents()) {
                    aiVector3D& srcTangent = srcMesh->mTangents[vertexIndex];
                    tmpTangent = { srcTangent.x, srcTangent.y, srcTangent.z };
                }
                else {
                    tmpTangent = GenerateTangent(tmpNormal);
                }

                if (srcMesh->HasTextureCoords(0)) {
                    aiVector3D& srcTexcoord = srcMesh->mTextureCoords[0][vertexIndex];
                    destVertex.texcood = { srcTexcoord.x, srcTexcoord.y };
                }
                else {
                    destVertex.texcood = Vector2::zero;
                }
                // 左手座標系に変換
                destVertex.position.z *= -1.0f;
                tmpNormal.z *= -1.0f;
                tmpTangent.z *= -1.0f;

                destVertex.normal = R32G32B32ToR10G10B10A2(tmpNormal);
                destVertex.tangent = R32G32B32ToR10G10B10A2(tmpTangent);
                vertices.emplace_back(destVertex);
            }

            destMesh.indexOffset = (uint32_t)indices.size();
            destMesh.indexCount = (uint32_t)srcMesh->mNumFaces * 3;
            for (uint32_t faceIndex = 0; faceIndex < srcMesh->mNumFaces; ++faceIndex) {
                aiFace& srcFace = srcMesh->mFaces[faceIndex];
                assert(srcFace.mNumIndices == 3);
                indices.emplace_back(srcFace.mIndices[0]);
                indices.emplace_back(srcFace.mIndices[2]);
                indices.emplace_back(srcFace.mIndices[1]);
            }

            for (uint32_t boneIndex = 0; boneIndex < srcMesh->mNumBones; ++boneIndex) {
                aiBone* bone = srcMesh->mBones[boneIndex];
                std::string jointName = bone->mName.C_Str();
                Model::JointWeightData& jointWeightData = skinClusterData[jointName];
                
                aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
                aiVector3D translate, scale;
                aiQuaternion rotate;
                bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
                Matrix4x4 bindPoseMatrix = Matrix4x4::MakeAffineTransform({ scale.x, scale.y, scale.z }, Quaternion{ -rotate.x, -rotate.y, rotate.z, rotate.w }, { translate.x, translate.y, -translate.z });
                jointWeightData.inverseBindPoseMatrix = bindPoseMatrix.Inverse();

                for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
                    jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId + destMesh.vertexOffset });
                }
            }

            // マテリアルが読み込まれてない
            assert(srcMesh->mMaterialIndex < materials.size());
            materials;
            destMesh.material = (uint32_t)srcMesh->mMaterialIndex;

            ++meshIndex;        
        }
        return meshes;
        
    }
    // aiSceneからPBRマテリアル配列を解析する
    std::vector<Material> ParseMaterials(const aiScene* scene, const std::filesystem::path& directory) {
        std::vector<Material> materials(scene->mNumMaterials);

        for (uint32_t materialIndex = 0; auto & destMaterial : materials) {
            const aiMaterial* srcMaterial = scene->mMaterials[materialIndex];

            aiColor3D albedo{};
            if (srcMaterial->Get(AI_MATKEY_BASE_COLOR, albedo) == aiReturn_SUCCESS) {
                destMaterial.albedo = { albedo.r, albedo.g, albedo.b };
            }
            float metallic{};
            if (srcMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == aiReturn_SUCCESS) {
                destMaterial.metallic = metallic;
            }
            float roughness{};
            if (srcMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == aiReturn_SUCCESS) {
                destMaterial.roughness = roughness;
            }
            aiColor3D emissive{};
            if (srcMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == aiReturn_SUCCESS) {
                destMaterial.emissive = { emissive.r, emissive.g, emissive.b };
            }
            float emissiveIntensity{};
            if (srcMaterial->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissiveIntensity) == aiReturn_SUCCESS) {
                destMaterial.emissiveIntensity = emissiveIntensity;
            }

            // テクスチャが一つ以上ある
            if (srcMaterial->GetTextureCount(aiTextureType_BASE_COLOR) > 0) {
                aiString path;
                srcMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &path);
                // 読み込む
                // TextureLoader内で多重読み込み対応済み
                std::string filename(path.C_Str());
                destMaterial.albedoMap = TextureLoader::Load(directory / filename);
            }
            // テクスチャが一つ以上ある
            if (srcMaterial->GetTextureCount(aiTextureType_METALNESS) > 0 &&
                srcMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0) {
                aiString metallicPath;
                aiString roughnessPath;
                srcMaterial->GetTexture(aiTextureType_METALNESS, 0, &metallicPath);
                srcMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessPath);
                // 同じテクスチャの場合使用
                if (metallicPath == roughnessPath) {
                    // 読み込む
                    // TextureLoader内で多重読み込み対応済み
                    std::string filename(metallicPath.C_Str());
                    destMaterial.metallicRoughnessMap = TextureLoader::Load(directory / filename, false);
                }
            }
            // テクスチャが一つ以上ある
            if (srcMaterial->GetTextureCount(aiTextureType_NORMALS) > 0) {
                aiString path;
                srcMaterial->GetTexture(aiTextureType_NORMALS, 0, &path);
                // 読み込む
                // TextureLoader内で多重読み込み対応済み
                std::string filename(path.C_Str());
                destMaterial.normalMap = TextureLoader::Load(directory / filename);
            }
            ++materialIndex;
        }
        return materials;
    }
    // 再起的にノードを解析する
    Node ParseNode(const aiNode* node) {
        Node result;
        aiVector3D translate, scale;
        aiQuaternion rotate;
        node->mTransformation.Decompose(scale, rotate, translate);
        result.transform.translate = { translate.x, translate.y, -translate.z };
        result.transform.rotate = Quaternion{ -rotate.x, -rotate.y, rotate.z, rotate.w };
        result.transform.scale = { scale.x, scale.y, scale.z };
        result.localMatrix = Matrix4x4::MakeAffineTransform(result.transform.scale, result.transform.rotate, result.transform.translate);
        result.name = node->mName.C_Str();
        // 子供も解析する
        result.children.resize(node->mNumChildren);
        for (uint32_t i = 0; i < node->mNumChildren; ++i) {
            result.children[i] = ParseNode(node->mChildren[i]);
        }
        return result;
    }


}

std::list<ModelInstance*> ModelInstance::instanceLists_;

std::shared_ptr<Model> Model::Load(const std::filesystem::path& path) {

    // privateコンストラクタをmake_sharedで呼ぶためのヘルパー
    struct Helper : Model {
        Helper() : Model() {}
    };
    std::shared_ptr<Model> model = std::make_shared<Helper>();

    auto directory = path.parent_path();
    Assimp::Importer importer;
    int flags = 0;

    // 三角形のみ
    flags |= aiProcess_Triangulate;
    // 左手座標系に変換
    flags |= aiProcess_FlipUVs;
    // 接空間を計算
    flags |= aiProcess_GenNormals;
    flags |= aiProcess_CalcTangentSpace;
    const aiScene* scene = importer.ReadFile(path.string(), flags);
    // 読み込めた
    if (!scene) {
        OutputDebugStringA(importer.GetErrorString());
        assert(false);
    }
    assert(scene->HasMeshes());

    model->materials_ = ParseMaterials(scene, directory);
    model->meshes_ = ParseMeshes(scene, model->materials_, model->vertices_, model->indices_, model->skinClusterData_);
    model->rootNode_ = ParseNode(scene->mRootNode);

    CommandContext commandContext;
    commandContext.Start(D3D12_COMMAND_LIST_TYPE_DIRECT);
    // 中間リソースをコピーする
    model->vertexBuffer_.Create(path.wstring() + L"VB", model->vertices_.size(), sizeof(model->vertices_[0]));
    model->indexBuffer_.Create(path.wstring() + L"IB", model->indices_.size(), sizeof(model->indices_[0]));

    commandContext.CopyBuffer(model->vertexBuffer_, model->vertexBuffer_.GetBufferSize(), model->vertices_.data());
    commandContext.CopyBuffer(model->indexBuffer_, model->indexBuffer_.GetBufferSize(), model->indices_.data());
    commandContext.TransitionResource(model->vertexBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
    commandContext.TransitionResource(model->indexBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
    commandContext.FlushResourceBarriers();

    // レイトレ用にBLASを作成
    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> blasDescs(model->meshes_.size());
    for (uint32_t meshIndex = 0; meshIndex < blasDescs.size(); ++meshIndex) {
        auto& mesh = model->meshes_[meshIndex];
        auto& desc = blasDescs[meshIndex];
        desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        desc.Triangles.VertexBuffer.StartAddress = model->vertexBuffer_.GetGPUVirtualAddress() + (uint64_t)mesh.vertexOffset * model->vertexBuffer_.GetElementSize();
        desc.Triangles.VertexBuffer.StrideInBytes = model->vertexBuffer_.GetElementSize();
        desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        desc.Triangles.VertexCount = mesh.vertexCount;
        desc.Triangles.IndexBuffer = model->indexBuffer_.GetGPUVirtualAddress() + (uint64_t)mesh.indexOffset * model->indexBuffer_.GetElementSize();
        desc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
        desc.Triangles.IndexCount = mesh.indexCount;

    }
    model->blas_.Create(L"ModelBLAS", commandContext, blasDescs);
    commandContext.Finish(true);

    return model;
}

ModelInstance::ModelInstance() {
    instanceLists_.emplace_back(this);
}

ModelInstance::~ModelInstance() {
    std::erase(instanceLists_, this);
    // auto iter = std::find(instanceLists_.begin(), instanceLists_.end(), this);
    // if (iter != instanceLists_.end()) {
    //     instanceLists_.erase(iter);
    // }
}
