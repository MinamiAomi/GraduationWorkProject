#pragma once
#include <memory>

#include "Engine/Graphics/Model.h"

#include "Math/MathUtils.h"
#include "Math/Transform.h"

#include "Collider.h"

class Trolley {
public:
	Trolley();

	void Initialize();
	void Update();

	bool UpdateCollision();

	std::shared_ptr<SphereCollider> GetCollider() { return chargerCollider_; }
	void SetTransform(const Transform& transform);
	float GetTrollySpeed() const { return trollySpeed_; }
private:
	void UpdateTrollySpeed();
	ModelInstance model_;

	Transform transform_;

	Vector3 trolleyOffset_;

#pragma region トロッコスピード関連
	float maxTrollySpeed_;
	float trollySpeed_;
	float trollyDeceleration_;
	float trollyAcceleration_;
	//MaxSpeed時のスピード維持時間
	int trollyFillUpTime_;
	int trollyMaxFillUpTime_;

	std::shared_ptr<SphereCollider> chargerCollider_;
	Vector3 chargerOffset_;
	float chargerRadius_;
#ifdef _DEBUG
	bool isDebugTrollySpeed_ = true;
#endif // _DEBUG
#pragma endregion
};