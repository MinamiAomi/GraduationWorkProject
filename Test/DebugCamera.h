#pragma once

#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Camera.h"

class DebugCamera {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	// ゲッター
	const std::shared_ptr<Camera>& GetCamera() const { return camera_; }
	//セッター
	void SetTransform(const Transform& t)
	{
		Vector3 forward = t.worldMatrix.GetForward();

		eulerAngle_.y = std::atan2(forward.x, forward.z);
		eulerAngle_.x = std::asin(-forward.y);
		eulerAngle_.z = 0.0f;

		Quaternion rotY = Quaternion::MakeFromAngleAxis(eulerAngle_.y, Vector3::up);
		Quaternion rotX = Quaternion::MakeFromAngleAxis(eulerAngle_.x, Vector3(1.0f, 0.0f, 0.0f));

		camera_->SetRotate(rotY * rotX);

		camera_->SetPosition(t.worldMatrix.GetTranslate());
		camera_->UpdateMatrices();
	}

private:
	std::shared_ptr<Camera> camera_;
	Vector3 eulerAngle_;

};