#include "Trolley.h"

#include "Framework/AssetManager.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG


Trolley::Trolley()
{
	model_.SetModel(AssetManager::GetInstance()->modelMap.Get("trolley")->Get());
	chargerCollider_ = std::make_shared<SphereCollider>(
		CollisionCategory::PLAYER,
		(CollisionCategory::FLASHLIGHT),
		Vector3::zero,
		0.0f
	);
}
void Trolley::Initialize()
{
	trolleyOffset_ = { 0.0f,-1.3f,0.0f };
	transform_.translate = trolleyOffset_;
	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);

	maxTrollySpeed_ = 1.0f;
	trollySpeed_ = 1.0f;
	trollyDeceleration_ = 0.001f;
	trollyAcceleration_ = 0.001f;
	trollyMaxFillUpTime_ = 30;
	trollyFillUpTime_ = trollyMaxFillUpTime_;


	chargerOffset_ = { 20.0f,-5.0f,0.0f };
	chargerRadius_ = 1.5f;
}

void Trolley::Update()
{

	bool isHit = UpdateCollision();
	if (!isHit) {
		UpdateTrollySpeed();
	}

	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);

#ifdef _DEBUG
	ImGui::Begin("GameScene");
	if (ImGui::TreeNode("Troller")) {
		if (ImGui::TreeNode("Troller")) {
			ImGui::DragFloat3("Offset", &trolleyOffset_.x, 0.01f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Charger")) {
			ImGui::DragFloat3("Offset", &chargerOffset_.x, 0.01f);
			ImGui::DragFloat("Radius", &chargerRadius_, 0.01f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("TrollerSpeed")) {
			ImGui::Checkbox("IsDebug", &isDebugTrollySpeed_);

			ImGui::Separator();

			ImGui::SliderFloat("CurrentTrollySpeed", &trollySpeed_, 0.0f, maxTrollySpeed_, "Speed: %.2f");
			ImGui::SliderInt("CurrentFillUpTime", &trollyFillUpTime_, 0, trollyMaxFillUpTime_, "Time: %d");

			ImGui::Separator();

			ImGui::DragFloat("MaxTrollySpeed", &maxTrollySpeed_, 0.01f);
			ImGui::DragFloat("Deceleration", &trollyDeceleration_, 0.001f);
			ImGui::DragFloat("Acceleration", &trollyAcceleration_, 0.001f);
			ImGui::DragInt("MaxFillUpTime", &trollyMaxFillUpTime_, 1);
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
	ImGui::End();
#endif // _DEBUG

}

void Trolley::UpdateTrollySpeed()
{
	//スピードがMaxだった場合
	if (trollyFillUpTime_ <= 0) {
#ifdef _DEBUG
		if (!isDebugTrollySpeed_) {

#endif // _DEBUG
			trollySpeed_ -= trollyDeceleration_;
			trollySpeed_ = std::clamp(trollySpeed_, 0.0f, maxTrollySpeed_);
#ifdef _DEBUG
		}
#endif // DEBUG
	}
	else {
		trollyFillUpTime_--;
	}
}

bool Trolley::UpdateCollision()
{
	bool result = false;
	if (!chargerCollider_->GetCollidedWith().empty()) {
		trollySpeed_ += trollyAcceleration_;
		trollySpeed_ = std::clamp(trollySpeed_, 0.0f, maxTrollySpeed_);
		result = true;
		//スピードがMaxかどうか
		if (trollySpeed_ >= maxTrollySpeed_) {
			trollyFillUpTime_ = trollyMaxFillUpTime_;
		}
	}
	return result;
}

void Trolley::SetTransform(const Transform& transform)
{
	transform_ = transform;

	chargerCollider_->center = transform_.translate + chargerOffset_;
	chargerCollider_->radius = chargerRadius_;

	transform_.translate += trolleyOffset_;
}
