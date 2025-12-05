#include "RailCameraSystem.h"

#include "Engine/File/JsonConverter.h"
#include "RailConverter.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

void RailSystem::RailCameraSystem::Initialize()
{
	JSON_OPEN("Resources/Data/RailCamera/railCameraSystem.json");
	JSON_LOAD(cameraOffset_);
	JSON_OBJECT("Fov");
	JSON_LOAD(baseFov_);
	JSON_LOAD(maxFov_);
	JSON_LOAD(fovLerpSpeed_);
	JSON_LOAD(referenceMaxSpeed_);
	JSON_ROOT();
	JSON_OBJECT("LookAhead");
	JSON_LOAD(futureFrame_);
	JSON_ROOT();
	JSON_CLOSE();

	Reset();
}

void RailSystem::RailCameraSystem::Reset()
{
	currentFov_ = baseFov_;

	transform_.translate = cameraOffset_;
	transform_.UpdateMatrix();
}

void RailSystem::RailCameraSystem::Update(float deltaTime)
{
	UpdateFov(deltaTime);
	UpdateLookAhead(deltaTime);

	transform_.translate = cameraOffset_;

	if (transform_.GetParent()) {
		Quaternion parentRotation = transform_.GetParent()->worldMatrix.GetRotate();
		transform_.rotate = parentRotation.Inverse() * currentLookRotation_;
	}
	else {
		transform_.rotate = currentLookRotation_;
	}

	transform_.UpdateMatrix();

#ifdef _DEBUG
	ImGui::Begin("GameScene");
	if (ImGui::TreeNode("RailCamera")) {
		if (ImGui::TreeNode("Offset")) {
			ImGui::DragFloat3("Offset", &cameraOffset_.x, 0.1f, 0.0f, 90.0f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/RailCamera/railCameraSystem.json");
				JSON_SAVE(cameraOffset_);
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Fov")) {
			ImGui::Text("MaxRef: %.2f", referenceMaxSpeed_);
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
				JSON_OPEN("Resources/Data/RailCamera/railCameraSystem.json");
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
		if (ImGui::TreeNode("LookAhead")) {
			ImGui::DragFloat("FutureFrame", &futureFrame_, 1.0f, 0.0f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/RailCamera/railCameraSystem.json");
				JSON_OBJECT("LookAhead");
				JSON_SAVE(futureFrame_);
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

void RailSystem::RailCameraSystem::UpdateFov(float deltaTime)
{

	deltaTime;
	float speedRatio = std::clamp(railCameraAnimationPlayer_->GetRealSpeed() / referenceMaxSpeed_, 0.0f, 1.0f);

	float t = speedRatio * speedRatio;

	float targetFov = Math::Lerp(t, baseFov_, maxFov_);

	float interpolationFactor = std::clamp(fovLerpSpeed_ * (1.0f / 60.0f), 0.0f, 1.0f);

	currentFov_ = std::lerp(currentFov_, targetFov, interpolationFactor);

}

void RailSystem::RailCameraSystem::UpdateLookAhead(float deltaTime)
{

	transform_.UpdateMatrix();
	Vector3 currentPos = transform_.worldMatrix.GetTranslate();
	Quaternion currentRotate = transform_.GetParent()->worldMatrix.GetRotate();
	float currentFrame = railCameraAnimationPlayer_->GetCurrentFrame();
	float futureFrame = currentFrame + futureFrame_;
	Vector3 targetPos = railCameraAnimationPlayer_->EvaluatePosition(futureFrame);

	Vector3 forwardVector = (targetPos - currentPos).Normalized();

	Vector3 upVector = Vector3::up;

	if (transform_.GetParent()) {
		Quaternion parentRotation = transform_.GetParent()->worldMatrix.GetRotate();
		upVector = parentRotation * Vector3::up;
	}


	Quaternion targetRotation = Quaternion::MakeLookRotation(forwardVector, upVector);

	float t = std::clamp(deltaTime * 5.0f, 0.0f, 1.0f);
	currentLookRotation_ = Quaternion::Slerp(t, currentLookRotation_, targetRotation);
}