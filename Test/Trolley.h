#pragma once
#include <memory>
#include <array>

#include "Engine/Graphics/Model.h"

#include "Graphics/LightManager.h"
#include "Audio/AudioSource.h"

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"

#include "Collider.h"
#include "RailAnimationPlayer.h"
#include "Flashlight.h"
#include "TrolleyUI.h"

class Trolley {
public:
	static const uint8_t BatteryNum = 1;

	static Trolley* GetInstance() {
		static Trolley instance;
		return &instance;
	}

	// コピーと代入を禁止
	Trolley(const Trolley&) = delete;
	Trolley& operator=(const Trolley&) = delete;

	enum class State {
		Normal,
		Overcharge,
		Nitro,
		Burst,
	};
	Trolley();

	void Initialize();
	void Update(float deltaTime);
	void Finalize();

	void SetParent(const Transform& transform) {
		transform_.SetParent(&transform);
		transform_.UpdateMatrix();
	}

	void SetBatteyParent(const Transform& transform) {
		for (uint8_t i = 0; i < BatteryNum; i++) {
			batteryTransforms_.at(i).SetParent(&transform);
			batteryTransforms_.at(i).UpdateMatrix();
		}
	}

	const Transform& GetTransform() { return transform_; }

	std::array<std::shared_ptr<SphereCollider>, BatteryNum> GetColliders() { return batteryColliders_; }

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

	const Transform& GetBatteyTransform(int i)const { return batteryTransforms_.at(i); }
	const Flashlight* GetFlashlight() const { return flashlight_; }

	const bool GetIsHitFlashlight()const { return isHitFlashlight_; }
	const bool GetBatteryRemaining() const { return !flashlight_->GetBatteryRemaining(); }
	const float GetCenterRate()const { return centerRate_; }

	void SetState(const State& state);

	const float GetBatteryRadius()const { return batteryRadius_; }

	void Pause() { isPause_ = true; }
	void Play() { isPause_ = false; }

	void SetIsActive(bool isActive);

	int batsNum_ = 0;

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
	void UpdateSound();

	//どこくらいライトの真ん中か計算
	float CalculateCenterRate(const Vector3& center, float radius);
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

#pragma region テールランプ
	/*std::shared_ptr<SpotLight> teilLight_;
	Transform teilLightTransform_;
	float teilLightRange_;
	float teilFovAngle_;
	float teilIntensity;
	Vector3 teilOffset_;*/
#pragma endregion

#pragma region トロッコスピード関連
	State trollyState_ = State::Normal;

	//いつのろのろ進むか
	float startFrame_;

	//通常時の最高速度
	float maxSpeed_;
	//最低スピード
	float minSpeed_;
	//バースト時の最高速度
	float burstSpeed_;
	//ニトロ時の最高速度
	float nitroSpeed_;
	// 光を当てた時の加速
	float accelerationRate_;
	// 自然減速
	float decelerationRate_;

	// 通常時のMAX
	float maxNormalCharge_;
	// ニトロ発動判定ライン
	float nitroThreshold_;
	// バースト発生ライン
	float burstThreshold_;

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

	float centerRate_ = 0.0f;

	float batDecrease_ = 1.0f;
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
	std::array<Transform, BatteryNum> batteryTransforms_;
	std::array<std::shared_ptr<SphereCollider>, BatteryNum> batteryColliders_;
	std::array<Vector3, BatteryNum> batteryOffsets_;
	float batteryRadius_;
#pragma endregion

#pragma region Shake
	Quaternion shakeRotation_;
	Vector3 shakeOffset_;
#pragma endregion

#pragma region Audio
	static constexpr uint32_t kNitroBoostSECount = 7;

	AudioSource normalSESource_;
	AudioSource nitroSESource_;
	AudioSource burstSESource_;
	AudioSource crashSESource_;
    AudioSource nitroBoostSESources_[7];
	AudioSource nitroFizzSESource_;
    AudioSource chargeSESource_;
#pragma endregion

	bool isPause_ = false;
#pragma endregion
};