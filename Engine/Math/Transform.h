#pragma once

#include "MathUtils.h"
#include "Graphics/ImGuiManager.h"

class Transform {
public:
    /// <summary>
    /// ワールド行列を更新
    /// </summary>
    void UpdateMatrix() {
        worldMatrix = Matrix4x4::MakeAffineTransform(scale, rotate, translate);
        if (parent_) {
            worldMatrix *= parent_->worldMatrix;
        }
    }

    /// <summary>
    /// 親をセット
    /// </summary>
    /// <param name="parent"></param>
    void SetParent(const Transform* parent, bool keepWorldTransform = true) {
        // 元々親がいた場合一度ワールド空間に戻す
        if (parent_ && keepWorldTransform) {
            scale = worldMatrix.GetScale();
            rotate = worldMatrix.GetRotate();
            translate = worldMatrix.GetTranslate();
        }
        parent_ = parent;
        // 新しい親がいる場合親空間のローカルにする
        if (parent_ && keepWorldTransform) {
            Matrix4x4 localMatrix = worldMatrix * parent_->worldMatrix.Inverse();
            scale = localMatrix.GetScale();
            rotate = localMatrix.GetRotate();
            translate = localMatrix.GetTranslate();
            return;
        }
    }

    void Debug(std::string name) {
        ImGui::DragFloat3((name + " Scale").c_str(), &scale.x, 0.1f);
        ImGui::DragFloat4((name + " Quaternion").c_str(), &rotate.x, 0.1f);
        ImGui::DragFloat3((name + " Translate").c_str(), &translate.x, 0.1f);
        UpdateMatrix();
    }

    const Transform* GetParent() const { return parent_; }

    Vector3 scale = Vector3::one;
    Quaternion rotate;
    Vector3 translate;
    Matrix4x4 worldMatrix;

private:
    const Transform* parent_ = nullptr;
};