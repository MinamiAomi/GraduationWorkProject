#pragma once

#include <memory>

#include "RailAnimationPlayer.h"
#include "Math/Transform.h"

namespace RailSystem {
	class RailCameraSystem {
	public:
		void Initialize();

		void Reset();

		void Update(float deltaTime);

		void SetRailAnimationPlayer(const RailSystem::RailAnimationPlayer* railCameraAnimationPlayer) { railCameraAnimationPlayer_ = railCameraAnimationPlayer; }
		void SetParent(const Transform& transform) { transform_ = transform; }
		float GetFov()const { return currentFov_; }
		const Transform& GetTransform()const { return transform_; }
		const Quaternion& GetLocalRotation() const { return transform_.rotate; }
		const Quaternion GetWorldRotation() const { return transform_.worldMatrix.GetRotate(); }
		const Vector3& GetLocalTranslate() const { return transform_.translate; }
		const Vector3 GetWorldTranslate() const { return transform_.worldMatrix.GetTranslate(); }
	private:
		void UpdateFov(float deltaTime);
		void UpdateLookAhead(float deltaTime);
		void UpdateBanking(float deltaTime);
		const RailSystem::RailAnimationPlayer* railCameraAnimationPlayer_;

		Vector3 cameraOffset_;
		Transform transform_;
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
		Quaternion currentLookRotation_;
		float futureFrame_;
#pragma endregion
#pragma region Banking
		Quaternion currentRotation_;
		float currentBankAngle_ = 0.0f;

		//バンクの強さ（大きいほど傾く）
		float bankingAmount_ = 30.0f;
		// 傾きの追従速度（ヌルヌル具合）
		float bankingSmoothTime_ = 5.0f;
		// 何フレーム先のカーブを読むか
		float lookAheadForBank_ = 20.0f;
#pragma endregion
	};
}