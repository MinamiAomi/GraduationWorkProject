#pragma once
#include <memory>

#include "Engine/Graphics/Model.h"

#include "Math/MathUtils.h"
#include "Math/Transform.h"

#include "Collider.h"

#include "Flashlight.h"
#include "TrolleyUI.h"

class Trolley {
public:
	Trolley();

	void Initialize();
	void Update();

	bool UpdateCollision();

	void SetParent(const Transform& transform) {
		transform_.SetParent(&transform);
		transform_.UpdateMatrix();
	}
	const Transform& GetTransform() { return transform_; }

	std::shared_ptr<SphereCollider> GetCollider() { return batteryCollider_; }

	//速度
	float GetTrollySpeed() const { return trollySpeed_; }
	//速度の割合(MAX = 1.0f)
	float GetTrollySpeedRatio() const { return trollySpeed_ / maxTrollySpeed_; }

	void SetFlashlight(const Flashlight* flashlight) { flashlight_ = flashlight; }
private:
	void UpdateTrollySpeed();
	void UpdateBanking();

	ModelInstance model_;

	Transform transform_;

	Vector3 trolleyOffset_;

	const Flashlight* flashlight_;

	TrolleyUI trolleyUI_;

#pragma region トロッコスピード関連
	float maxTrollySpeed_;
	float trollySpeed_;
	float trollyDeceleration_;
	float trollyAcceleration_;
	//MaxSpeed時のスピード維持時間
	int trollyFillUpTime_;
	int trollyMaxFillUpTime_;
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

#pragma region Battery
	Transform batteryTransform_;
	std::shared_ptr<SphereCollider> batteryCollider_;
	Vector3 batteryOffset_;
	float batteryRadius_;
#pragma endregion

#ifdef _DEBUG
	bool isDebugTrollySpeed_ = true;
#endif // _DEBUG
#pragma endregion
};