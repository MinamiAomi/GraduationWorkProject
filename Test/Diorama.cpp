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

void Diorama::Initialize(const std::string& name, const Vector3& position)
{
#ifdef _DEBUG
	name_ = name;
#endif // _DEBUG
	auto assetManager = AssetManager::GetInstance();
	model_.SetModel(assetManager->modelMap.Get(name)->Get());

	transform_.translate = position;

	model_.SetWorldMatrix(transform_.worldMatrix);
	collider_->center = transform_.worldMatrix.GetTranslate();
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
	static float rotationY = 0.0f;
	rotationY += 0.005f;

	transform_.rotate = Quaternion::MakeForYAxis(rotationY);
	transform_.UpdateMatrix();
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