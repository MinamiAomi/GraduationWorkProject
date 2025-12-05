#pragma once

#include <string>
#include <list>
#include <memory>

#include "Math/MathUtils.h"

class DirectionalLight {
public:
    DirectionalLight();
    void DrawImGui(const std::string& label);

    Color color;
    Vector3 direction;
    float intensity;
    bool isActive;
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
    bool isActive;
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
    bool isActive;
    bool useVolumeLight;
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

    const std::vector<std::weak_ptr<DirectionalLight>>& GetActiveDirectionalLights() const { return activeDirectionalLights_; }
    const std::vector<std::weak_ptr<PointLight>>& GetActivePointLights() const { return activePointLights_; }
    const std::vector<std::weak_ptr<SpotLight>>& GetActiveSpotLights() const { return activeSpotLights_; }

private:
    std::list<std::weak_ptr<DirectionalLight>> directionalLights_;
    std::list<std::weak_ptr<PointLight>> pointLights_;
    std::list<std::weak_ptr<SpotLight>> spotLights_;

    std::vector<std::weak_ptr<DirectionalLight>> activeDirectionalLights_;
    std::vector<std::weak_ptr<PointLight>> activePointLights_;
    std::vector<std::weak_ptr<SpotLight>> activeSpotLights_;

};