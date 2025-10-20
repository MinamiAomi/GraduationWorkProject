#pragma once

#include <string>
#include <vector>

#include "Math/MathUtils.h"


class DirectionalLight {
public:
    DirectionalLight() : color(Vector3::one), direction(Vector3::down), intensity(1.0f) {}
    void DrawImGui(const std::string& label);

    Vector3 color;
    Vector3 direction;
    float intensity;
};

//class PointLight {
//public:
//    PointLight() : color(Vector3::one), position(Vector3::zero), intensity(0.0f), radius(1.0f), decay(1.0f) {};
//    void DrawImGui(const std::string& label);
//
//    Vector3 color;
//    Vector3 position;
//    float intensity;
//    float radius;
//    float decay;
//};
//
//class SpotLight {
//public:
//    SpotLight() : color(Vector3::one), position
//
//};
//
//class LightManager {
//public:
//    void Add(const std::shared_ptr<DirectionalLight>& light) { directionalLights_.emplace_back(light); }
//    void Add(const std::shared_ptr<PointLight>& light) { pointLights_.emplace_back(light); }
//    void Add(const std::shared_ptr<SpotLight>& light) { spotLights_.emplace_back(light); }
//
//    const std::vector<std::shared_ptr<DirectionalLight>>& GetDirectionalLights() const { return directionalLights_; }
//    const std::vector<std::shared_ptr<PointLight>>& GetPointLights() const { return pointLights_; }
//    const std::vector<std::shared_ptr<SpotLight>>& GetSpotLights() const { return spotLights_; }
//
//    void Reset() {
//        directionalLights_.clear();
//        pointLights_.clear();
//        spotLights_.clear();
//    }
//
//private:
//    std::vector<std::shared_ptr<DirectionalLight>> directionalLights_;
//    std::vector<std::shared_ptr<PointLight>> pointLights_;
//    std::vector<std::shared_ptr<SpotLight>> spotLights_;
//};