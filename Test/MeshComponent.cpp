#include "MeshComponent.h"

#include "GameObject/GameObject.h"

#include "Framework/AssetManager.h"
#include "Graphics/ImGuiManager.h"

void MeshComponent::Initialize() {
    if (!modelName_.empty()) {
        auto assetManager = AssetManager::GetInstance();
        asset_ = assetManager->modelMap.Get(modelName_);
        ApplyModel();
    }

    auto gameObject = GetGameObject();
    gameObject->transform.UpdateMatrix();
    model_.SetWorldMatrix(gameObject->transform.worldMatrix);

}

void MeshComponent::Update() {
    model_.SetWorldMatrix(GetGameObject()->transform.worldMatrix);
}

void MeshComponent::Edit() {
#ifdef ENABLE_IMGUI

    if (ImGui::BeginCombo("Model", asset_->GetName().c_str())) {
        AssetManager::GetInstance()->modelMap.ForEach([this](const std::shared_ptr<ModelAsset>& asset) {
            bool isSelected = asset->GetName() == asset_->GetName();
            if (ImGui::Selectable(asset->GetName().c_str(), isSelected) && asset->IsReady()) {
                asset_ = asset;
                modelName_ = asset_->GetName();
                ApplyModel();
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
            });
        ImGui::EndCombo();
    }

    if (customMaterial_) {
        ImGui::ColorEdit3("Albedo", &customMaterial_->albedo.x);
        ImGui::SliderFloat("Metallic", &customMaterial_->metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &customMaterial_->roughness, 0.0f, 1.0f);
        ImGui::ColorEdit3("Emissive", &customMaterial_->emissive.x);
        ImGui::DragFloat("EmissiveIntensity", &customMaterial_->emissiveIntensity, 0.1f);
        bool refraction = model_.Refraction();
        ImGui::Checkbox("Refraction", &refraction);
        model_.SetRefraction(refraction);
        bool alphaTest = model_.AlphaTest();
        ImGui::Checkbox("AlphaTest", &alphaTest);
        model_.SetAlphaTest(alphaTest);
    }
#endif // ENABLE_IMGUI
}

void MeshComponent::Export(nlohmann::json& json) const {
    if (!modelName_.empty()) {
        json["model"] = modelName_;
    }
}

void MeshComponent::Import(const nlohmann::json& json) {
    if (json.contains("model")) {
        modelName_ = json.at("model").get<std::string>();
    }
}

void MeshComponent::ApplyModel() {
    assert(asset_);
    model_.SetModel(asset_->Get());
    customMaterial_ = std::shared_ptr<Material>();

    if (model_.GetModel()->GetMaterials().size() <= 2) {
        customMaterial_ = std::make_shared<Material>(model_.GetModel()->GetMaterials()[0]);
    }
    model_.SetMaterial(customMaterial_);
}
