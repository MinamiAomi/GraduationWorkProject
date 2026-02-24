#pragma once

#include <string>

#include "Math/Transform.h"
#include "Math/MathUtils.h"
#include "Graphics/Model.h"
#include "Collider.h"

class Diorama {
public:
	enum RotateAxis {
		XAxis, 
		YAxis,
		ZAxis
	};

	Diorama();

	void Initialize(const std::string& name, const Vector3& position, const Quaternion& initialRotate = Quaternion::identity);

	void Update();

	std::shared_ptr<SphereCollider> GetCollider() { return collider_; }
    void SetRoateAxis(RotateAxis axis) { rotateAxis_ = axis; }
	void SetRotateRate(float rotateRate) { rotateRate_ = rotateRate; }
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
	Quaternion initialRotate_;
    RotateAxis rotateAxis_ = YAxis;
	float maxCount_ = 180.0f;
	float count_ = 0.0f;
	float rotateRate_ = 1.0f;
	float rotationY_ = 0.0f;
};