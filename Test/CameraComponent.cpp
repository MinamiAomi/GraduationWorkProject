#include "CameraComponent.h"

#include "GameObject/GameObject.h"

#include "Input/Input.h"
#include "Graphics/ImGuiManager.h"
#include "Graphics/RenderManager.h"

CameraComponent::CameraComponent() {
    camera_ = std::make_shared<Camera>();
}

void CameraComponent::Initialize() {

    auto gameObject = GetGameObject();
    camera_->SetPosition(gameObject->transform.translate);
    camera_->SetRotate(gameObject->transform.rotate);
    euler_ = camera_->GetRotate().EulerAngle();
    camera_->UpdateMatrices();
    RenderManager::GetInstance()->SetCamera(camera_);
}

void CameraComponent::Update() {

    auto gameObject = GetGameObject();

    MoveCamera();
    gameObject->transform.translate = camera_->GetPosition();
    gameObject->transform.rotate = camera_->GetRotate();
    camera_->UpdateMatrices();
}

void CameraComponent::Edit() {
}

void CameraComponent::Export(nlohmann::json& json) const {
    json;
}

void CameraComponent::Import(const nlohmann::json& json) {
    json;
}

void CameraComponent::MoveCamera() {
    Input* input = Input::GetInstance();

    auto mouseMoveX = input->GetMouseMoveX();
    auto mouseMoveY = input->GetMouseMoveY();
    auto wheel = input->GetMouseWheel();

    Quaternion rotate = camera_->GetRotate();
    Vector3 position = camera_->GetPosition();

    Vector3 diffPosition;

    if (input->IsMousePressed(1)) {
        constexpr float rotSpeed = Math::ToRadian * 0.1f;
        euler_.x += rotSpeed * static_cast<float>(mouseMoveY);
        euler_.y += rotSpeed * static_cast<float>(mouseMoveX);
    }
    else if (input->IsMousePressed(2)) {
        Vector3 cameraX = rotate.GetRight() * (-static_cast<float>(mouseMoveX) * 0.01f);
        Vector3 cameraY = rotate.GetUp() * (static_cast<float>(mouseMoveY) * 0.01f);
        diffPosition += cameraX + cameraY;
    }
    else if (wheel != 0) {
        Vector3 cameraZ = rotate.GetForward() * (static_cast<float>(wheel / 120) * 0.5f);
        diffPosition += cameraZ;
    }

    {
        auto BoolInt = [](bool x) {
            return x ? 1 : 0;
            };

        int xRotate = BoolInt(input->IsKeyPressed(DIK_DOWN)) - BoolInt(input->IsKeyPressed(DIK_UP));
        int yRotate = BoolInt(input->IsKeyPressed(DIK_RIGHT)) - BoolInt(input->IsKeyPressed(DIK_LEFT));

        constexpr float rotSpeed = Math::ToRadian * 1.0f;
        euler_.x += rotSpeed * static_cast<float>(xRotate);
        euler_.y += rotSpeed * static_cast<float>(yRotate);


        int xMove = BoolInt(input->IsKeyPressed(DIK_D)) - BoolInt(input->IsKeyPressed(DIK_A));
        Vector3 cameraX = rotate.GetRight() * (float)xMove * 0.5f;
        diffPosition += cameraX;

        int yMove = BoolInt(input->IsKeyPressed(DIK_SPACE)) - BoolInt(input->IsKeyPressed(DIK_LSHIFT));
        Vector3 cameraY = rotate.GetUp() * (float)yMove * 0.5f;
        diffPosition += cameraY;

        int zMove = BoolInt(input->IsKeyPressed(DIK_W)) - BoolInt(input->IsKeyPressed(DIK_S));
        Vector3 cameraZ = rotate.GetForward() * (float)zMove * 0.5f;
        diffPosition += cameraZ;
    }



    camera_->SetPosition(position + diffPosition);
    camera_->SetRotate(Quaternion::MakeFromEulerAngle(euler_));
}
