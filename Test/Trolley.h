#pragma once
#include <memory>

#include "Engine/Graphics/Model.h"

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"

#include "Collider.h"
#include "RailAnimationPlayer.h"
#include "Flashlight.h"
#include "TrolleyUI.h"

class Trolley {
public:
	enum class State {
		Normal,
		Overcharge,
		Nitro,
		Burst,
	};
	Trolley();

	void Initialize();
	void Update(float deltaTime);


	void SetParent(const Transform& transform) {
		transform_.SetParent(&transform);
		transform_.UpdateMatrix();
	}
	const Transform& GetTransform() { return transform_; }

	std::shared_ptr<SphereCollider> GetCollider() { return batteryCollider_; }

	//速度
	float GetTrollySpeed() const { return currentSpeed_; }
	//速度の割合(MAX = 1.0f)
	float GetTrollySpeedRatio() const { return currentSpeed_ / maxSpeed_; }

	float GetCurrentCharge() const { return currentCharge_; }
	float GetMaxNormalCharge() const { return maxNormalCharge_; }

	float GetBurstThreshold() const { return burstThreshold_; }

	float GetNitroAccumulateTimer() const { return nitroAccumulateTimer_; }
	float GetNitroChargeTime() const { return nitroChargeTime_; }

	void SetFlashlight(const Flashlight* flashlight) { flashlight_ = flashlight; }
	void SetRailAnimationPlayer(const RailSystem::RailAnimationPlayer* railCameraAnimationPlayer) { railCameraAnimationPlayer_ = railCameraAnimationPlayer; }

	const State& GetState()const { return trollyState_; }
private:
	void UpdateCollision();
	void UpdateState(float deltaTime);
	void UpdateBanking(float deltaTime);

	void OnNormalState();
	void OnOverchargeState();
	void OnNitroState();
	void RecoverFromNitro();
	void OnBurstState();
	void RecoverFromBurst();
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG
	Random::RandomNumberGenerator rnd_;
	const RailSystem::RailAnimationPlayer* railCameraAnimationPlayer_;

	ModelInstance model_;

	Transform transform_;

	Vector3 trolleyOffset_;

	const Flashlight* flashlight_;

	TrolleyUI trolleyUI_;

#pragma region トロッコスピード関連
	State trollyState_ = State::Normal;

	//通常時の最高速度
	float maxSpeed_ = 1.0f;
	//バースト時の最高速度
	float burstSpeed_ = 0.3f;
	//ニトロ時の最高速度
	float nitroSpeed_ = 1.5f;
	// 光を当てた時の加速
	float accelerationRate_ = 1.0f;
	// 自然減速
	float decelerationRate_ = 0.5f;

	// 通常時のMAX
	float maxNormalCharge_ = 100.0f;
	// ニトロ発動判定ライン
	float nitroThreshold_ = 140.0f;
	// バースト発生ライン
	float burstThreshold_ = 150.0f;

	// ニトロ発動に必要な維持時間
	float nitroChargeTime_ = 3.0f;
	// ニトロ持続時間
	float nitroDuration_ = 3.0f;
	// バースト演出時間
	float burstDuration_ = 2.0f;

	// ニトロ終了後のバッテリー量
	float batteryAfterNitro_ = 80.0f;
	// バースト後のバッテリー量
	float batteryAfterBurst_ = 30.0f;

	float currentSpeed_ = 0.0f;
	// バッテリー残量
	float currentCharge_ = 100.0f;

	// ニトロ発動条件を満たしている時間累積
	float nitroAccumulateTimer_ = 0.0f;
	// 現在のステートに滞在している時間
	float stateTimer_ = 0.0f;

	bool isHitFlashlight_ = false;
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

#pragma region Shake
	Quaternion shakeRotation_;
#pragma endregion


#ifdef _DEBUG
	bool isDebugTrollySpeed_ = false;
#endif // _DEBUG
#pragma endregion
};