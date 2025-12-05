#include "Trolley.h"

#include "Framework/AssetManager.h"

#include "Engine/File/JsonConverter.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG


Trolley::Trolley()
{
	model_.SetModel(AssetManager::GetInstance()->modelMap.Get("trolley")->Get());
	batteryCollider_ = std::make_shared<SphereCollider>(
		CollisionCategory::PLAYER,
		(CollisionCategory::FLASHLIGHT),
		Vector3::zero,
		0.0f
	);
}
void Trolley::Initialize()
{
	JSON_OPEN("Resources/Data/Trolley/trolley.json");
	JSON_OBJECT("TrollerSpeed");
	JSON_LOAD(maxTrollySpeed_);
	JSON_LOAD(trollyDeceleration_);
	JSON_LOAD(trollyAcceleration_);
	JSON_LOAD(trollyMaxFillUpTime_);
	JSON_ROOT();
	JSON_OBJECT("Trolley");
	JSON_LOAD(trolleyOffset_);
	JSON_ROOT();
	JSON_OBJECT("Battery");
	JSON_LOAD(batteryOffset_);
	JSON_LOAD(batteryRadius_);
	JSON_ROOT();
	JSON_OBJECT("Banking");
	JSON_LOAD(bankingAmount_);
	JSON_LOAD(bankingSmoothTime_);
	JSON_LOAD(lookAheadForBank_);
	JSON_ROOT();
	JSON_CLOSE();

	transform_.translate = trolleyOffset_;
	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);

	trollyFillUpTime_ = trollyMaxFillUpTime_;
	trollySpeed_ = maxTrollySpeed_;

	batteryTransform_.translate = batteryOffset_;
	batteryTransform_.SetParent(&transform_);
	batteryTransform_.UpdateMatrix();

	batteryCollider_->center = batteryTransform_.worldMatrix.GetTranslate();
	batteryCollider_->radius = batteryRadius_;

	trolleyUI_.Initialize(transform_);
	trolleyUI_.SetTrolley(this);
}

void Trolley::Update()
{

	bool isHit = UpdateCollision();
	if (!isHit) {
		UpdateTrollySpeed();
	}

	UpdateBanking();

	Quaternion bankRotation = Quaternion::MakeFromAngleAxis(currentBankAngle_, Vector3(0.0f, 0.0f, 1.0f));

	transform_.translate = trolleyOffset_;
	transform_.rotate = bankRotation;
	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);

	batteryTransform_.UpdateMatrix();
	batteryCollider_->center = batteryTransform_.worldMatrix.GetTranslate();

	trolleyUI_.Update();

#ifdef _DEBUG
	ImGui::Begin("TrolleySpeed");
	ImGui::Checkbox("IsDebug", &isDebugTrollySpeed_);

	ImGui::Separator();

	bool isFillingUp = (trollyFillUpTime_ > 0);

	if (isFillingUp) {
		ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.5f, 0.2f, 0.5f));

		ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: FILLING UP! (%.1fs)", float(trollyFillUpTime_) / 60.0f);
	}

	ImGui::SliderFloat("Speed", &trollySpeed_, 0.0f, maxTrollySpeed_, "%.2f km/h");

	if (isFillingUp) {
		ImGui::PopStyleColor(2);
	}

	ImGui::End();

	ImGui::Begin("GameScene");
	if (ImGui::TreeNode("Troller")) {
		if (ImGui::TreeNode("Troller")) {
			ImGui::DragFloat3("Offset", &trolleyOffset_.x, 0.01f);

			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Trolley/trolley.json");
				JSON_OBJECT("Trolley");
				JSON_SAVE(trolleyOffset_);
				JSON_ROOT();
				JSON_CLOSE();
			}

			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Battery")) {
			ImGui::DragFloat3("Offset", &batteryOffset_.x, 0.01f);
			ImGui::DragFloat("Radius", &batteryRadius_, 0.01f);
			batteryTransform_.translate = batteryOffset_;
			batteryTransform_.UpdateMatrix();
			batteryCollider_->radius = batteryRadius_;
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Trolley/trolley.json");
				JSON_OBJECT("Battery");
				JSON_SAVE(batteryOffset_);
				JSON_SAVE(batteryRadius_);
				JSON_ROOT();
				JSON_CLOSE();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("TrollerSpeed")) {
			ImGui::SliderFloat("CurrentTrollySpeed", &trollySpeed_, 0.0f, maxTrollySpeed_, "Speed: %.2f");
			ImGui::SliderInt("CurrentFillUpTime", &trollyFillUpTime_, 0, trollyMaxFillUpTime_, "Time: %d");

			ImGui::Separator();

			ImGui::DragFloat("MaxTrollySpeed", &maxTrollySpeed_, 0.01f);
			ImGui::DragFloat("Deceleration", &trollyDeceleration_, 0.001f);
			ImGui::DragFloat("Acceleration", &trollyAcceleration_, 0.001f);
			ImGui::DragInt("MaxFillUpTime", &trollyMaxFillUpTime_, 1);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Trolley/trolley.json");
				JSON_OBJECT("TrollerSpeed");
				JSON_SAVE(maxTrollySpeed_);
				JSON_SAVE(trollyDeceleration_);
				JSON_SAVE(trollyAcceleration_);
				JSON_SAVE(trollyMaxFillUpTime_);
				JSON_ROOT();
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Banking")) {
			ImGui::DragFloat("Amount", &bankingAmount_, 1.0f, 0.0f);
			ImGui::DragFloat("SmoothTime", &bankingSmoothTime_, 1.0f, 0.0f);
			ImGui::DragFloat("LookAheadForBank", &lookAheadForBank_, 1.0f, 0.0f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Trolley/Trolley.json");
				JSON_OBJECT("Banking");
				JSON_SAVE(bankingAmount_);
				JSON_SAVE(bankingSmoothTime_);
				JSON_SAVE(lookAheadForBank_);
				JSON_ROOT();
				JSON_CLOSE();

			}
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

void Trolley::UpdateBanking()
{
	/*float currentFrame = railCameraAnimationPlayer_->GetCurrentFrame();

	Vector3 posNow = railCameraAnimationPlayer_->EvaluatePosition(currentFrame);
	Vector3 posFuture = railCameraAnimationPlayer_->EvaluatePosition(currentFrame + lookAheadForBank_);

	Vector3 forwardNow = currentLookRotation_ * Vector3(0, 0, 1);

	Quaternion blenderRotation = railCameraAnimationPlayer_->EvaluateRotation(currentFrame);
	Vector3 railUpVector = blenderRotation * Vector3(0, 1, 0);

	Vector3 dirToFuture = (posFuture - posNow).Normalized();

	Vector3 curveCross = Vector3::Cross(forwardNow, dirToFuture);

	float turnIntensity = Vector3::Dot(curveCross, railUpVector);

	float targetBankAngle = -turnIntensity * currentRealSpeed_ * bankingAmount_;

	targetBankAngle = std::clamp(targetBankAngle, -45.0f * Math::ToRadian, 45.0f * Math::ToRadian);

	currentBankAngle_ = std::lerp(currentBankAngle_, targetBankAngle, deltaTime * bankingSmoothTime_);*/
}

bool Trolley::UpdateCollision()
{
	bool result = false;
	if (!batteryCollider_->GetCollidedWith().empty()) {
		for (const auto& collider : batteryCollider_->GetCollidedWith()) {
			if (collider->categoryBits == CollisionCategory::FLASHLIGHT) {
				//フラッシュライトが点灯しているか
				if (flashlight_->GetIsLighting()) {
					trollySpeed_ += trollyAcceleration_;
					trollySpeed_ = std::clamp(trollySpeed_, 0.0f, maxTrollySpeed_);
					result = true;
					//スピードがMaxかどうか
					if (trollySpeed_ >= maxTrollySpeed_) {
						trollyFillUpTime_ = trollyMaxFillUpTime_;
					}
				}
			}
		}
	}
	return result;
}
