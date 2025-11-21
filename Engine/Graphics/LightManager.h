#pragma once

#include <string>
#include <list>
#include <memory>

#include "Math/MathUtils.h"

class DirectionalLight {
public:
    DirectionalLight();
    void DrawImGui(const std::string& label);

    Vector3 color;
    Vector3 direction;
    float intensity;
};

class PointLight {
public:
    PointLight();
    void DrawImGui(const std::string& label);

    Vector3 color;
    Vector3 position;
    float intensity;
    float radius;
    float decay;
};

class SpotLight {
public:
    SpotLight();
    void DrawImGui(const std::string& label);

    Vector3 color;
    Vector3 position;
    Vector3 direction;
    float intensity;
    float distance;
    float angle;
    float falloffStartAngle;
    float decay;
};

class LightManager {
public:
    void Add(const std::shared_ptr<DirectionalLight>& light);
    void Add(const std::shared_ptr<PointLight>& light);
    void Add(const std::shared_ptr<SpotLight>& light);

    void Remove(const std::shared_ptr<DirectionalLight>& light);
    void Remove(const std::shared_ptr<PointLight>& light);
    void Remove(const std::shared_ptr<SpotLight>& light);

    void Reset();
    
    const std::list<std::shared_ptr<DirectionalLight>>& GetDirectionalLights() const { return directionalLights_; }
    const std::list<std::shared_ptr<PointLight>>& GetPointLights() const { return pointLights_; }
    const std::list<std::shared_ptr<SpotLight>>& GetSpotLights() const { return spotLights_; }

private:
    std::list<std::shared_ptr<DirectionalLight>> directionalLights_;
    std::list<std::shared_ptr<PointLight>> pointLights_;
    std::list<std::shared_ptr<SpotLight>> spotLights_;

};