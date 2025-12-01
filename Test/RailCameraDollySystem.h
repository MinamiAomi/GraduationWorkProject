#pragma once

#include <memory>

#include "RailCameraAnimationPlayer.h"

namespace RailCameraSystem {
	class RailCameraDollySystem {
	public:
		void Initialize();

		void Reset();

		void Update(float deltaTime);

		void SetRailCameraAnimationPlayer(const RailCameraSystem::RailCameraAnimationPlayer* railCameraAnimationPlayer) { railCameraAnimationPlayer_ = railCameraAnimationPlayer; }

		float GetFov()const { return currentFov_; }
		Quaternion GetRotation() const { return currentRotation_; }
	private:
		void UpdateFov(float deltaTime);
		void UpdateLookAhead(float deltaTime);

		const RailCameraSystem::RailCameraAnimationPlayer* railCameraAnimationPlayer_;
#pragma region FOV
		float currentFov_;
		float baseFov_ = 45.0f * Math::ToRadian;
		float maxFov_ = 90.0f * Math::ToRadian;
		//変化の追従速度(大きいほどきびきび、小さいほどぬるぬる)
		float fovLerpSpeed_ = 5.0f;
		Vector3 preCameraPosition_;
		float referenceMaxSpeed_ = 30.0f;
		float currentRealSpeed_ = 0.0f;
#pragma endregion
#pragma region Look Ahead
		Quaternion currentRotation_;
#pragma endregion


	};
}