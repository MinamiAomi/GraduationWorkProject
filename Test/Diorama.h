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

	bool GetIsActive() { return isActive_; }
private:
	bool OnCollision();
#ifdef _DEBUG
	void DrawImGui();
	std::string name_;
#endif // _DEBUG
	ModelInstance model_;
	std::shared_ptr<SphereCollider> collider_;
	Transform transform_;
	bool isActive_;
	float maxCount_ = 120.0f;
	float count_ = 0.0f;
};