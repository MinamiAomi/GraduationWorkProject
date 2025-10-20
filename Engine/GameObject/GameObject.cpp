#include "GameObject.h"

#include "Graphics/ImGuiManager.h"

void GameObject::RenderInInspectorView() {
#ifdef ENABLE_IMGUI
    ImGui::PushID(this);
    ImGui::Checkbox("##IsActive", &isActive_);
    ImGui::SameLine();
    ImGui::InputText("##Name", &name_, ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Separator();
        ImGui::DragFloat3("Translate", &transform.translate.x, 0.1f);
        Vector3 rotate = transform.rotate.EulerAngle() * Math::ToDegree;
        ImGui::DragFloat3("Rotate", &rotate.x, 1.0f);
        transform.rotate = Quaternion::MakeFromEulerAngle(rotate * Math::ToRadian);
        ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
        transform.UpdateMatrix();
        ImGui::TreePop();
    }
    for (auto& component : componentList_) {
        ImGui::Separator();
        if (ImGui::TreeNodeEx(component.second->GetComponentName().c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Separator();
            component.second->Edit();
            ImGui::TreePop();
        }
    }
    ImGui::PopID();
#endif // ENABLE_IMGUI
}

void GameObject::InitializeUninitializedComponents() {
    // 未初期化のコンポーネントを初期化
    for (auto& component : uninitializedComponents_) {
        component->Initialize();
    }
    uninitializedComponents_.clear();

    for (const auto& child : children_) {
        if (auto sp = child.lock()) {
            sp->InitializeUninitializedComponents();
        }
    }
}

void GameObject::Update() {
    // すべてのコンポーネントを更新
    for (auto& component : componentList_) {
        component.second->Update();
    }
    transform.UpdateMatrix();

    for (const auto& child : children_) {
        if (auto sp = child.lock()) {
            sp->Update();
        }
    }
}

void GameObject::SetParent(const std::shared_ptr<GameObject>& gameObject) {
    // 親がもともといる場合
    if (auto currentParent = parent_.lock()) {
        // 子供を削除
        currentParent->RemoveChild(shared_from_this());
    }
    // 新しい親がいる場合
    parent_ = gameObject;
    if (gameObject) {
        gameObject->AddChild(shared_from_this());

    }
}

void GameObject::AddChild(const std::shared_ptr<GameObject>& gameObject) {
    children_.push_back(gameObject);
}

void GameObject::RemoveChild(const std::shared_ptr<GameObject>& gameObject) {
    children_.erase(std::remove_if(children_.begin(), children_.end(), [&](const std::weak_ptr<GameObject>& child) {
        return child.lock() == gameObject;
        }),
        children_.end());
}
