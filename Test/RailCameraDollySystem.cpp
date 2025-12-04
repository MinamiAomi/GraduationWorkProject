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
	JSON_OBJECT("Banking");
	JSON_LOAD(bankingAmount_);
	JSON_LOAD(bankingSmoothTime_);
	JSON_LOAD(lookAheadForBank_);
	JSON_ROOT();
	JSON_CLOSE();

	Reset();
}

void RailCameraSystem::RailCameraDollySystem::Reset()
{
	currentRealSpeed_ = 0.0f;
	currentFov_ = baseFov_;

	preCameraPosition_ = railCameraAnimationPlayer_->GetCurrentTransform().translate;

	currentBankAngle_ = 0.0f;
}

void RailCameraSystem::RailCameraDollySystem::Update(float deltaTime)
{
	UpdateFov(deltaTime);
	//UpdateLookAhead(deltaTime);
	//UpdateBanking(deltaTime);
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

		if (ImGui::TreeNode("Banking")) {
			ImGui::DragFloat("Amount", &bankingAmount_, 1.0f, 0.0f);
			ImGui::DragFloat("SmoothTime", &bankingSmoothTime_, 1.0f, 0.0f);
			ImGui::DragFloat("LookAheadForBank", &lookAheadForBank_, 1.0f, 0.0f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/RailCamera/dollySystem.json");
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

void RailCameraSystem::RailCameraDollySystem::UpdateLookAhead(float deltaTime)
{
	Vector3 currentPosition = railCameraAnimationPlayer_->GetCurrentTransform().translate;

	float lookAheadFrames = 30.0f;
	float futureFrame = railCameraAnimationPlayer_->GetCurrentFrame() + lookAheadFrames;

	Vector3 targetPosition = railCameraAnimationPlayer_->EvaluatePosition(futureFrame);

	Vector3 forwardVector = (targetPosition - currentPosition).Normalized();

	Quaternion targetRotation = Quaternion::MakeLookRotation(forwardVector);

	float rotationSmoothness = 5.0f * deltaTime;
	currentRotation_ = Quaternion::Slerp(rotationSmoothness, currentRotation_, targetRotation);
}

void RailCameraSystem::RailCameraDollySystem::UpdateBanking(float deltaTime)
{
	float currentFrame = railCameraAnimationPlayer_->GetCurrentFrame();

	Transform currentTrans = railCameraAnimationPlayer_->GetCurrentTransform();
	Vector3 myPosition = currentTrans.translate;
	Quaternion baseRotation = currentTrans.rotate;

	Vector3 posNow = railCameraAnimationPlayer_->EvaluatePosition(currentFrame);
	Vector3 posFuture = railCameraAnimationPlayer_->EvaluatePosition(currentFrame + lookAheadForBank_);

	Vector3 forwardNow = baseRotation * Vector3::unitZ;

	Vector3 dirToFuture = (posFuture - posNow).Normalized();

	Vector3 curveCross = Vector3::Cross(forwardNow, dirToFuture);

	float turnIntensity = curveCross.y;

	float targetBankAngle = -turnIntensity * currentRealSpeed_ * bankingAmount_;

	targetBankAngle = std::clamp(targetBankAngle, -45.0f * Math::ToRadian, 45.0f * Math::ToRadian);

	currentBankAngle_ = std::lerp(currentBankAngle_, targetBankAngle, deltaTime * bankingSmoothTime_);
	Quaternion bankRotation = Quaternion::MakeFromAngleAxis(currentBankAngle_, Vector3(0.0f, 0.0f, 1.0f));
	currentRotation_ =  bankRotation* baseRotation;

}
