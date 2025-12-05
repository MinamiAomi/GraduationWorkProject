#pragma once
#include <memory>

#include "Engine/Graphics/Model.h"

#include "Math/MathUtils.h"
#include "Math/Transform.h"

#include "Collider.h"

#include "RailAnimationPlayer.h"
#include "Flashlight.h"
#include "TrolleyUI.h"

class Trolley {
public:
	Trolley();

	void Initialize();
	void Update(float deltaTime);

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
	void SetRailAnimationPlayer(const RailSystem::RailAnimationPlayer* railCameraAnimationPlayer) { railCameraAnimationPlayer_ = railCameraAnimationPlayer; }
private:
	void UpdateTrollySpeed();
	void UpdateBanking(float deltaTime);

	const RailSystem::RailAnimationPlayer* railCameraAnimationPlayer_;

	ModelInstance model_;

	Transform transform_;

	Vector3 trolleyOffset_;

	const Flashlight* flashlight_;

	TrolleyUI trolleyUI_;

#pragma region トロッコスピード関連

	float maxBatteryTrollySpeed_;
	float maxTrollySpeed_;
	float trollySpeed_;
	float trollyDeceleration_;
	float trollyAcceleration_;

	//過充電後の充電量
	float afterOverchargeBattery_ = 30.0f;
	float afterNitroBattery_ = 80.0f;

	//バーストするまでの時間
	float maxBurstTime_ = 300.0f;
	//過充電ニトロまでの時間
	float maxOverchargeTime_ = 180.0f;

	//MaxSpeed時のスピード維持時間
	int trollyFillUpTime_;
	//オーバーチャージしちゃう量
	int trollyMaxOverChargeFillUpTime_;
	//何秒間過充電するか
	int trollyMaxFillUpTime_;
#pragma endregion

#pragma region Banking
	Quaternion currentRotation_;
	float currentBankAngle_ = 0.0f;

	//バンクの強さ（大きいほど傾く）
	float bankingAmount_;
	// 傾きの追従速度（ヌルヌル具合）
	float bankingSmoothTime_;
	// 何フレーム先のカーブを読むか
	float lookAheadForBank_;
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