#pragma once

#include <memory>

#include "RailAnimationPlayer.h"
#include "Math/Transform.h"
#include "Math/Random.h"

namespace RailSystem {
	class RailCameraSystem {
	public:
		void Initialize();

		void Reset();

		void Update(float deltaTime);

		void SetRailAnimationPlayer(const RailSystem::RailAnimationPlayer* railCameraAnimationPlayer) { railCameraAnimationPlayer_ = railCameraAnimationPlayer; }
		void SetParent(const Transform& transform) { transform_.SetParent(&transform); }

		void SetCameraShake(float time) {
			isCameraShake_ = true;
			cameraShakeTime_ = time;
		}

		float GetFov()const { return currentFov_; }
		const Transform& GetTransform()const { return transform_; }
		const Quaternion& GetLocalRotation() const { return transform_.rotate; }
		const Quaternion GetWorldRotation() const { return transform_.worldMatrix.GetRotate(); }
		const Vector3& GetLocalTranslate() const { return transform_.translate; }
		const Vector3 GetWorldTranslate() const { return transform_.worldMatrix.GetTranslate(); }
	private:
		void UpdateFov(float deltaTime);
		void UpdateLookAhead(float deltaTime);
		void UpdateCameraShake();
#ifdef _DEBUG
		void DrawImGui();
#endif // _DEBUG

		const RailSystem::RailAnimationPlayer* railCameraAnimationPlayer_;

		Vector3 pointOfGazeOffset_;
		Vector3 cameraOffset_;
		Transform transform_;
#pragma region FOV
		float currentFov_;
		float baseFov_ = 45.0f * Math::ToRadian;
		float maxFov_ = 90.0f * Math::ToRadian;
		//変化の追従速度(大きいほどきびきび、小さいほどぬるぬる)
		float fovLerpSpeed_ = 5.0f;
		float referenceMaxSpeed_ = 30.0f;
#pragma endregion
#pragma region Look Ahead
		Quaternion currentLookRotation_;
		float futureFrame_;
#pragma endregion
#pragma region CameraShake
		Random::RandomNumberGenerator rnd_;
		bool isCameraShake_;
		float cameraShakeTime_;
		float shakeRange_;
		Vector3 shakeOffset_;
#pragma endregion

	};
}