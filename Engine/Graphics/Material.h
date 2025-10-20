#pragma once

#include <memory>
#include <string>

#include "Math/MathUtils.h"
#include "Core/TextureResource.h"
#include "Core/UploadBuffer.h"

class Material {
public:
    Vector3 albedo = { 0.8f, 0.8f, 0.8f };
    float metallic = 0.0f;
    float roughness = 0.5f;
    Vector3 emissive = { 0.0f, 0.0f, 0.0f };
    float emissiveIntensity = 1.0f;
    std::shared_ptr<TextureResource> albedoMap;
    std::shared_ptr<TextureResource> metallicRoughnessMap;
    std::shared_ptr<TextureResource> normalMap;
};