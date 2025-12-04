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

	std::shared_ptr<SphereCollider> batteryCollider_;
	Vector3 batteryOffset_;
	float batteryRadius_;
#ifdef _DEBUG
	bool isDebugTrollySpeed_ = true;
#endif // _DEBUG
#pragma endregion
};