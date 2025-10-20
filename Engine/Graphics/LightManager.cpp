#include "LightManager.h"

#include "ImGuiManager.h"

void DirectionalLight::DrawImGui(const std::string& label) {
    (void)label;
#ifdef ENABLE_IMGUI
    bool openTree = ImGui::TreeNode(label.c_str());
    if (openTree) {
        ImGui::ColorEdit3("Color", &color.x);
        ImGui::DragFloat3("Direction", &direction.x, 0.01f, -1.0f, 1.0f);
        direction = direction.Normalized();
        ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 100.0f);
        ImGui::TreePop();
    }
#endif //ENABLE_IMGUI
}
