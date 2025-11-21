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
		eulerAngle_ = t.rotate.EulerAngle();
		camera_->SetPosition(t.translate);
		camera_->SetRotate(Quaternion::MakeFromEulerAngle(eulerAngle_));
		camera_->UpdateMatrices();
	}

private:
	std::shared_ptr<Camera> camera_;
	Vector3 eulerAngle_;

};