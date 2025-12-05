#include "LightManager.h"

#include "ImGuiManager.h"

DirectionalLight::DirectionalLight() : color(Color::white), direction(Vector3::down), intensity(1.0f) {}

void DirectionalLight::DrawImGui(const std::string& label) {
    (void)label;
#ifdef ENABLE_IMGUI
    bool openTree = ImGui::TreeNode(label.c_str());
    if (openTree) {
        ImGui::ColorEdit3("Color", color.GetAddress());
        ImGui::DragFloat3("Direction", &direction.x, 0.01f, -1.0f, 1.0f);
        direction = direction.Normalized();
        ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 100.0f);
        ImGui::TreePop();
    }
#endif //ENABLE_IMGUI
}

PointLight::PointLight() : color(Color::white), position(Vector3::zero), intensity(1.0f), range(1.0f), decay(1.0f) {}

void PointLight::DrawImGui(const std::string& label) {
    (void)label;
#ifdef ENABLE_IMGUI
    bool openTree = ImGui::TreeNode(label.c_str());
    if (openTree) {
        ImGui::ColorEdit3("Color", color.GetAddress());
        ImGui::DragFloat3("Position", &position.x, 0.1f);
        ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 100.0f);
        ImGui::DragFloat("decay", &intensity, 0.1f, 0.0f, 10.0f);
        ImGui::TreePop();
    }
#endif //ENABLE_IMGUI
}

SpotLight::SpotLight() :
    color(Color::white), position(Vector3::zero), direction(-Vector3::unitY),
    intensity(1.0f), range(1.0f), angle(60.0f * Math::ToRadian),
    falloffStartAngle(45.0f * Math::ToRadian), decay(1.0f) {
}

void SpotLight::DrawImGui(const std::string& label) {
    (void)label;
#ifdef ENABLE_IMGUI
    bool openTree = ImGui::TreeNode(label.c_str());
    if (openTree) {
        ImGui::ColorEdit3("Color", color.GetAddress());
        ImGui::DragFloat3("Position", &position.x, 0.1f);
        ImGui::DragFloat3("Direction", &direction.x, 0.01f, -1.0f, 1.0f);
        direction = direction.Normalized();
        ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 100.0f);
        ImGui::DragFloat("Distance", &range, 0.01f, 0.1f, 100.0f);
        float angleDegree = angle * Math::ToDegree;
        ImGui::DragFloat("Angle", &angleDegree, 0.1f, 0.0f, 180.0f);
        angle = angleDegree * Math::ToRadian;
        float falloffStartAngleDegree = falloffStartAngle * Math::ToDegree;
        ImGui::DragFloat("FalloffStartAngle", &falloffStartAngleDegree, 0.1f, 0.0f, angleDegree);
        falloffStartAngle = falloffStartAngleDegree * Math::ToRadian;
        ImGui::DragFloat("Decay", &intensity, 0.1f, 0.0f, 10.0f);
        ImGui::TreePop();
    }
#endif //ENABLE_IMGUI
}

void LightManager::Add(const std::shared_ptr<DirectionalLight>& light) { directionalLights_.emplace_back(light); }
void LightManager::Add(const std::shared_ptr<PointLight>& light) { pointLights_.emplace_back(light); }
void LightManager::Add(const std::shared_ptr<SpotLight>& light) { spotLights_.emplace_back(light); }

void LightManager::Reset() {
    directionalLights_.clear();
    pointLights_.clear();
    spotLights_.clear();
}

void LightManager::UpdateActiveLights(const Camera& camera) {
    activePointLights_.clear();
    activeSpotLights_.clear();

    for (auto it = directionalLights_.begin(); it != directionalLights_.end();) {
        if (auto light = it->lock()) {
            ++it;
        }
        else {
            it = directionalLights_.erase(it);
        }

    }

    for (auto it = pointLights_.begin(); it != pointLights_.end();) {
        if (auto light = it->lock()) {
            if (camera.GetFrustum().Intersects({ light->position, light->range })) {
                activePointLights_.push_back(light);
            }
            ++it;
        }
        else {
            it = pointLights_.erase(it);
        }
    }

    for (auto it = spotLights_.begin(); it != spotLights_.end();) {
        if (auto light = it->lock()) {
            if (camera.GetFrustum().InersectsSpotLight(light->position, light->direction, light->range, light->angle)) {
                activeSpotLights_.push_back(light);
            }
            ++it;
        }
        else {
            it = spotLights_.erase(it);
        }
    }

}