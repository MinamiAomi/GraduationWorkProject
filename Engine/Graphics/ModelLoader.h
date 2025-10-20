#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>

#include "Math/MathUtils.h"

struct ModelData {
    // 頂点
    struct Vertex {
        Vector3 position;
        Vector3 normal;
        Vector2 texcoord;
    };
    
    // インデックス
    using Index = uint16_t;
    // メッシュ
    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
        uint32_t materialIndex = 0;
    };
    // マテリアル
    struct Material {
        std::string name;
        Vector3 diffuse = { 0.8f,0.8f, 0.8f };
        Vector3 specular = { 0.5f,0.5f,0.5f };
        uint32_t textureIndex = 0;
    };
    struct Texture {
        std::filesystem::path filePath;
    };


    static ModelData LoadObjFile(const std::filesystem::path& path);

    std::string name;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<Texture> textures;
};
