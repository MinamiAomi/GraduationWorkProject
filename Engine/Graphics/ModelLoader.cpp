#include "ModelLoader.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <optional>

#include "assimp/scene.h"
#include "assimp/Importer.hpp"

namespace {
    void LoadMTLFile(ModelData& modelData, const std::filesystem::path& path) {
        std::ifstream file(path);
        assert(file.is_open());

        std::filesystem::path parentPath = path.parent_path();

        std::vector<ModelData::Material>& materials = modelData.materials;
        std::vector<ModelData::Texture>& textures = modelData.textures;
        ModelData::Material* currentMaterial = nullptr;

        std::string line;
        while (std::getline(file, line)) {
            std::string identifier;
            std::istringstream iss(line);
            iss >> identifier;

            // コメントをスキップ
            if (identifier == "#") {
                continue;
            }
            else if (identifier == "newmtl") {
                std::string materialName;
                iss >> materialName;
                currentMaterial = &materials.emplace_back();
                currentMaterial->name = materialName;
            }
            else if (identifier == "map_Kd") {
                std::string textureName;
                iss >> textureName;
                std::filesystem::path textureFilePath = parentPath / textureName;
                auto iter = std::find_if(textures.begin(), textures.end(),
                    [&](const auto& texture) {
                        return texture.filePath == textureFilePath;
                    });
                if (iter != textures.end()) {
                    currentMaterial->textureIndex = uint32_t(std::distance(textures.begin(), iter));
                }
                else {
                    auto& texture = textures.emplace_back();
                    texture.filePath = textureFilePath;
                    currentMaterial->textureIndex = uint32_t(textures.size() - 1);
                }
            }
            else if (identifier == "Kd") {
                assert(currentMaterial);
                Vector3 diffuse;
                iss >> diffuse.x >> diffuse.y >> diffuse.z;
                currentMaterial->diffuse = diffuse;
            }
            else if (identifier == "Ks") {
                assert(currentMaterial);
                Vector3 specular;
                iss >> specular.x >> specular.y >> specular.z;
                currentMaterial->specular = specular;
            }
        }
    }

}

ModelData ModelData::LoadObjFile(const std::filesystem::path& path) {
    ModelData modelData;
    std::ifstream file(path);
    assert(file.is_open());

    std::filesystem::path parentPath = path.parent_path();

    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;

    Mesh* currentMesh = nullptr;
    // 同じ組み合わせの頂点をインデックスに対応させるため
    std::unordered_map<std::string, Index> vertexDefinitionMap;

    std::string line;
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream iss(line);
        iss >> identifier;

        // コメント
        if (identifier == "#") {
            continue;
        }
        // マテリアルを読み込む
        else if (identifier == "mtllib") {
            std::string materialFileName;
            iss >> materialFileName;
            LoadMTLFile(modelData, parentPath / materialFileName);
        }
        // 座標
        else if (identifier == "v") {
            Vector3& position = positions.emplace_back();
            iss >> position.x >> position.y >> position.z;
            position.x = -position.x;
        }
        // 法線
        else if (identifier == "vn") {
            Vector3& normal = normals.emplace_back();
            iss >> normal.x >> normal.y >> normal.z;
            normal.x = -normal.x;
        }
        // UV座標
        else if (identifier == "vt") {
            Vector2& texcoord = texcoords.emplace_back();
            iss >> texcoord.x >> texcoord.y;
            texcoord.y = 1.0f - texcoord.y;
        }
        // 新しいマテリアル
        else if (identifier == "usemtl") {
            std::string materialName;
            iss >> materialName;
            // マテリアルは配列から名前が一致する物を探す
            auto iter = std::find_if(modelData.materials.begin(), modelData.materials.end(),
                [&](const auto& material) {
                    return material.name == materialName;
                });
            // 見つからないはずがない
            assert(iter != modelData.materials.end());
            // 新しいメッシュを始める
            currentMesh = &modelData.meshes.emplace_back();
            currentMesh->materialIndex = uint32_t(std::distance(modelData.materials.begin(), iter));
            // 頂点かぶりはメッシュごと
            vertexDefinitionMap.clear();
        }
        // 面情報
        else if (identifier == "f") {
            assert(currentMesh != nullptr);
            // 面の頂点を取得
            std::vector<std::string> vertexDefinitions;
            // 面の頂点は3つ以上
            while (true) {
                std::string vertexDefinition;
                iss >> vertexDefinition;
                if (vertexDefinition.empty()) {
                    break;
                }
                vertexDefinitions.emplace_back(std::move(vertexDefinition));
            }
            // 面は三角形から
            assert(vertexDefinitions.size() >= 3);
            // 面を構成するインデックス
            std::vector<Index> face(vertexDefinitions.size());
            for (uint32_t i = 0; i < vertexDefinitions.size(); ++i) {
                // 頂点が登録済み
                if (vertexDefinitionMap.contains(vertexDefinitions[i])) {
                    // 登録済みの頂点インデックスを持ってくる
                    face[i] = vertexDefinitionMap[vertexDefinitions[i]];
                }
                else {
                    // 頂点を生成
                    std::istringstream viss(vertexDefinitions[i]);
                    const uint32_t kInvalidElement = uint32_t(-1);
                    uint32_t elementIndices[3]{ kInvalidElement };
                    for (uint32_t j = 0; j < 3; ++j) {
                        std::string index;
                        std::getline(viss, index, '/');
                        if (!index.empty()) {
                            elementIndices[j] = static_cast<uint32_t>(std::stoi(index)) - 1;
                        }
                    }
                    Vertex vertex;
                    vertex.position = positions[elementIndices[0]];
                    if (elementIndices[1] != kInvalidElement) { vertex.texcoord = texcoords[elementIndices[1]]; }
                    if (elementIndices[2] != kInvalidElement) { vertex.normal = normals[elementIndices[2]]; }
                    currentMesh->vertices.emplace_back(vertex);
                    // 頂点インデックスを登録
                    face[i] = vertexDefinitionMap[vertexDefinitions[i]] = static_cast<Index>(currentMesh->vertices.size() - 1);
                }
            }
            // 読み込んだポリゴンを三角形リスト形式で格納していく
            for (uint32_t i = 0; i < face.size() - 2; ++i) {
                auto& indices = currentMesh->indices;
                indices.emplace_back(Index(face[0]));
                indices.emplace_back(Index(face[i + 1ull]));
                indices.emplace_back(Index(face[i + 2ull]));
            }
        }
    }

    return modelData;
}
