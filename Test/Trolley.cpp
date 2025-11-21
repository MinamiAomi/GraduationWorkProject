#include "Trolley.h"

#include "Framework/AssetManager.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG


Trolley::Trolley()
{
	model_.SetModel(AssetManager::GetInstance()->modelMap.Get("trolley")->Get());
}

void Trolley::Initialize()
{
	offset_ = { 0.0f,-1.3f,0.0f };
	transform_.translate += offset_;
	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);

	maxTrollySpeed_ = 1.0f;
	trollySpeed_ = 1.0f;
	trollyDeceleration_ = 0.001f;
	trollyAcceleration_ = 0.001f;
	trollyMaxFillUpTime_ = 30;
	trollyFillUpTime_ = trollyMaxFillUpTime_;
}

void Trolley::Update()
{

	TrollySpeedUpdate();

	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);

#ifdef _DEBUG
	ImGui::Begin("GameScene");
	if (ImGui::TreeNode("Troller")) {
		if (ImGui::TreeNode("Troller")) {
			ImGui::DragFloat3("Offset", &offset_.x, 0.01f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("TrollerSpeed")) {

			ImGui::SliderFloat("CurrentTrollySpeed", &trollySpeed_, 0.0f, maxTrollySpeed_, "Speed: %.2f");
			ImGui::SliderInt("CurrentFillUpTime", &trollyFillUpTime_, 0, trollyFillUpTime_, "Time: %d");

			ImGui::Separator();

			ImGui::DragFloat("MaxTrollySpeed", &maxTrollySpeed_, 0.01f);
			ImGui::DragFloat("Deceleration", &trollyDeceleration_, 0.001f);
			ImGui::DragFloat("Acceleration", &trollyAcceleration_, 0.001f);
			ImGui::DragInt("MaxFillUpTime", &trollyFillUpTime_, 1);
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
	ImGui::End();
#endif // _DEBUG

}

void Trolley::TrollySpeedUpdate()
{
	//スピードがMaxだった場合
	if (trollyFillUpTime_ <= 0) {
		trollySpeed_ -= trollyDeceleration_;
		trollySpeed_ = std::clamp(trollySpeed_, 0.0f, maxTrollySpeed_);
	}
	else {
		trollyFillUpTime_--;
	}
}

void Trolley::OnCollision()
{
	trollySpeed_ += trollyAcceleration_;
	trollySpeed_ = std::clamp(trollySpeed_, 0.0f, maxTrollySpeed_);
	//スピードがMaxかどうか
	if (trollySpeed_ >= maxTrollySpeed_) {
		trollyFillUpTime_ = trollyMaxFillUpTime_;
	}
}

void Trolley::SetTransform(const Transform& transform)
{
	transform_ = transform;
	transform_.translate += offset_;
}
