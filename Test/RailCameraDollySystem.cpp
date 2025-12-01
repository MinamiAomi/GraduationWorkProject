#include "RailCameraDollySystem.h"

#include "Engine/File/JsonConverter.h"


#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

void RailCameraSystem::RailCameraDollySystem::Initialize()
{
	JSON_OPEN("Resources/Data/RailCamera/dollySystem.json");
	JSON_OBJECT("Fov");
	JSON_LOAD(baseFov_);
	JSON_LOAD(maxFov_);
	JSON_LOAD(fovLerpSpeed_);
	JSON_LOAD(referenceMaxSpeed_);
	JSON_ROOT();
	JSON_CLOSE();

	Reset();
}

void RailCameraSystem::RailCameraDollySystem::Reset()
{
	currentRealSpeed_ = 0.0f;
	currentFov_ = baseFov_;

	preCameraPosition_ = railCameraAnimationPlayer_->GetCurrentTransform().translate;
}

void RailCameraSystem::RailCameraDollySystem::Update(float deltaTime)
{
	UpdateFov(deltaTime);

#ifdef _DEBUG
	ImGui::Begin("GameScene");
	if (ImGui::TreeNode("Dolly")) {

		if (ImGui::TreeNode("Fov")) {
			ImGui::Text("Real Speed: %.2f / MaxRef: %.2f", currentRealSpeed_, referenceMaxSpeed_);
			ImGui::Spacing();
			ImGui::Separator();
			float baseFov = baseFov_ * Math::ToDegree;
			float maxFov = maxFov_ * Math::ToDegree;
			ImGui::DragFloat("BaseFov", &baseFov, 1.0f, 0.0f, 90.0f);
			ImGui::DragFloat("MaxFov", &maxFov, 1.0f, baseFov, 90.0f);
			ImGui::DragFloat("LerpSpeed", &fovLerpSpeed_, 1.0f, 0.0f);
			ImGui::DragFloat("ReferenceMaxSpeed", &referenceMaxSpeed_, 1.0f, 0.0f);
			baseFov_ = baseFov * Math::ToRadian;
			maxFov_ = maxFov * Math::ToRadian;
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/RailCamera/dollySystem.json");
				JSON_OBJECT("Fov");
				JSON_SAVE(baseFov_);
				JSON_SAVE(maxFov_);
				JSON_SAVE(fovLerpSpeed_);
				JSON_SAVE(referenceMaxSpeed_);
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

void RailCameraSystem::RailCameraDollySystem::UpdateFov(float deltaTime)
{
	Vector3 currentPos = railCameraAnimationPlayer_->GetCurrentTransform().translate;

	if (deltaTime > 0.0001f) {
		float distance = (currentPos - preCameraPosition_).Length();
		currentRealSpeed_ = distance / deltaTime;
	}
	else {
		currentRealSpeed_ = 0.0f;
	}

	preCameraPosition_ = currentPos;


	// 空間上の速度(currentRealSpeed_) を基準速度(referenceMaxSpeed_)で割る
		// これで「実際の見た目の速さ」に対する割合が出ます
	float speedRatio = std::clamp(currentRealSpeed_ / referenceMaxSpeed_, 0.0f, 1.0f);

	float t = speedRatio * speedRatio;

	float targetFov = Math::Lerp(t, baseFov_, maxFov_);

	float interpolationFactor = std::clamp(fovLerpSpeed_ * (1.0f / 60.0f), 0.0f, 1.0f);

	currentFov_ = std::lerp(currentFov_, targetFov, interpolationFactor);

}
