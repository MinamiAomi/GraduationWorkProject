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
		camera_->SetPosition(t.translate);
		camera_->SetRotate(t.rotate);
	}
private:
	std::shared_ptr<Camera> camera_;
};