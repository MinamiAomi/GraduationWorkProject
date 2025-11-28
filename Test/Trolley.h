#pragma once
#include <memory>

#include "Engine/Graphics/Model.h"

#include "Math/MathUtils.h"
#include "Math/Transform.h"

#include "Collider.h"

#include "Flashlight.h"

class Trolley {
public:
	Trolley();

	void Initialize();
	void Update();

	bool UpdateCollision();

	std::shared_ptr<SphereCollider> GetCollider() { return batteryCollider_; }
	void SetTransform(const Transform& transform);
	float GetTrollySpeed() const { return trollySpeed_; }
	void SetFlashlight(const Flashlight* flashlight) { flashlight_ = flashlight; }
private:
	void UpdateTrollySpeed();
	ModelInstance model_;

	Transform transform_;

	Vector3 trolleyOffset_;

	const Flashlight* flashlight_;

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