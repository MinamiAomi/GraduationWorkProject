#pragma once
#include "Engine/Graphics/Model.h"

#include "Math/MathUtils.h"
#include "Math/Transform.h"

class Trolley {
public:
	Trolley();

	void Initialize();
	void Update();
	void TrollySpeedUpdate();

	void OnCollision();

	void SetTransform(const Transform& transform);
	float GetTrollySpeed() const { return trollySpeed_; }
private:
	ModelInstance model_;

	Transform transform_;

	Vector3 offset_;

#pragma region トロッコスピード関連
	float maxTrollySpeed_;
	float trollySpeed_;
	float trollyDeceleration_;
	float trollyAcceleration_;
	//MaxSpeed時のスピード維持時間
	int trollyFillUpTime_;
	int trollyMaxFillUpTime_;
#pragma endregion
};