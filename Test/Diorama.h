#pragma once

#include <string>

#include "Math/Transform.h"
#include "Math/MathUtils.h"
#include "Graphics/Model.h"
#include "Collider.h"

class Diorama {
public:
	Diorama();

	void Initialize(const std::string& name, const Vector3& position);

	void Update();

	std::shared_ptr<SphereCollider> GetCollider() { return collider_; }

	bool GetIsActive() { return count_>= maxCount_; }
private:
	bool OnCollision();
#ifdef _DEBUG
	void DrawImGui();
	std::string name_;
#endif // _DEBUG
	ModelInstance model_;
	std::shared_ptr<SphereCollider> collider_;
	Transform transform_;
	float maxCount_ = 240.0f;
	float count_ = 0.0f;
	float rotationY_ = 0.0f;
};