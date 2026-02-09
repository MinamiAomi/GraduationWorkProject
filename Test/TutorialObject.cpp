#include "TutorialObject.h"

#include "Framework/AssetManager.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

TutorialObject::TutorialObject()
{
	collider_ = std::make_shared<SphereCollider>(
		CollisionCategory::TUTORIAL,
		CollisionCategory::FLASHLIGHT,
		Vector3::zero,
		0.0f
	);
}

void TutorialObject::Initialize(const Transform& transform, const std::string& name)
{
	transform_ = transform;
	transform_.UpdateMatrix();

	collider_->center = transform_.translate;
	collider_->radius = 1.0f;
#ifdef _DEBUG
	name_ = name;
#endif // _DEBUG

	auto assetManager = AssetManager::GetInstance();

	auto model = assetManager->modelMap.Get(name)->Get();
	auto texture = assetManager->textureMap.Get(name)->Get();

	model_.SetModel(model);

	model_.SetWorldMatrix(transform_.worldMatrix);

	sprite_.SetTexture(texture);
	sprite_.SetColor(Color::white);
	sprite_.SetPosition({ 1280.0f * 0.5f, 720.0f * 0.5f });
	sprite_.SetScale(texture->GetSize());
	sprite_.SetAnchor({ 0.5f, 0.5f });
	sprite_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	sprite_.SetIsActive(false);
	sprite_.SetDrawOrder(5);

}

void TutorialObject::Update()
{
	transform_.UpdateMatrix();

	collider_->center = transform_.translate;

	model_.SetWorldMatrix(transform_.worldMatrix);
	OnCollision();

#ifdef _DEBUG
	DrawImGui();
#endif // _DEBUG

}

void TutorialObject::OnCollision()
{
	if (!collider_->GetCollidedWith().empty()) {
		sprite_.SetIsActive(true);
	}
	else {
		sprite_.SetIsActive(false);   
	}
}

#ifdef _DEBUG
void TutorialObject::DrawImGui()
{
	ImGui::Begin(name_.c_str());
	ImGui::DragFloat3("translate", &transform_.translate.x);
	Vector3 euler = transform_.rotate.EulerAngle();
	ImGui::DragFloat3("rotation", &euler.x);
	ImGui::DragFloat("radius", &collider_->radius);
	transform_.rotate = Quaternion::MakeFromEulerAngle(euler);
	ImGui::End();

	sprite_.DrawImGui(name_ + "sprite");
}
#endif // _DEBUG
