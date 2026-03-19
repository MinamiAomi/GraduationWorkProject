#include "Diorama.h"

#include "Framework/AssetManager.h"

Diorama::Diorama()
{
    collider_ = std::make_shared<SphereCollider>(
        CollisionCategory::DIORAMA,
        CollisionCategory::FLASHLIGHT,
        Vector3::zero,
        0.0f
    );
}

void Diorama::Initialize(const std::string& name, const Vector3& position, const Quaternion& initialRotate)
{
#ifdef _DEBUG
    name_ = name;
#endif // _DEBUG
    auto assetManager = AssetManager::GetInstance();
    model_.SetModel(assetManager->modelMap.Get(name)->Get());

    transform_.translate = position;
    transform_.rotate = initialRotate;
    transform_.UpdateMatrix();

    initialRotate_ = initialRotate;


    model_.SetWorldMatrix(transform_.worldMatrix);
    collider_->center = transform_.worldMatrix.GetTranslate();
    collider_->radius = 0.8f;
    count_ = 0.0f;

}

void Diorama::Update()
{
    if (OnCollision()) {
        count_++;
    }
    else {
        count_ = 0.0f;
    }

    rotationY_ += 0.005f * rotateRate_ + count_ * 0.001f;
    switch (rotateAxis_)
    {
    case XAxis:
        transform_.rotate = initialRotate_ * Quaternion::MakeForXAxis(rotationY_);
        break;
    case YAxis:
        transform_.rotate = initialRotate_ * Quaternion::MakeForYAxis(rotationY_);
        break;
    case ZAxis:
        transform_.rotate = initialRotate_ * Quaternion::MakeForZAxis(rotationY_);
        break;
    default:
        break;
    }
    transform_.UpdateMatrix();
    collider_->center = transform_.worldMatrix.GetTranslate();
    model_.SetWorldMatrix(transform_.worldMatrix);

#ifdef _DEBUG
    DrawImGui();
#endif // _DEBUG

}

bool Diorama::OnCollision()
{
    if (!collider_->GetCollidedWith().empty()) {
        return true;
    }
    return false;
}

#ifdef _DEBUG
void Diorama::DrawImGui()
{
    ImGui::Begin(name_.c_str());
    ImGui::DragFloat3("translate", &transform_.translate.x);
    ImGui::DragFloat3("scale", &transform_.scale.x);
    ImGui::DragFloat("radius", &collider_->radius);
    ImGui::End();
}
#endif // _DEBUG