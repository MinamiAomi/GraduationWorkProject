#pragma once

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Camera.h"
#include "Graphics/Model.h"

class Flashlight {
public:
	void Initialize(const Transform* parentTransform, const Camera* parentCamera);
	void Update();

	Vector3 GetDirection() const;
	Vector3 GetPosition() const;
	Vector3 GetFocusPoint() const { return focusPoint_; }

private:
	const float MinFocusDistance = 300.0f;
	const float MaxFocusDistance = 1000.0f;
	const float MinFocusRange = 100.0f;
	const float MaxFocusRange = 500.0f;
	
	//移動
	void Move();
	//マウスカーソルのライト移動
	void UpdateFocusPoint();
	//ライトの形状変更
	void UpdateLightShape();
    void SpotLightDebugDraw() const;
	
	Vector3 UnprojectScreenToWorld(
		const Vector2& mousePos, const Vector2& screenArea) const;
	void SetProperties(float fovAngleDegrees, float range);

	const Transform* parentTransform_ = nullptr;
	const Camera* parentCamera_ = nullptr;
	
	ModelInstance lightModel_;
    Transform lightTransform_;
	// カメラからの距離
	float distanceFromCamera_ = 10.0f;
    // カメラの球面を移動するための角度x
	float sphericalAngleX_ = 0.0f;
	// カメラの球面を移動するための角度y
    float sphericalAngleY_ = 0.0f;
	// ライトの視野角度
	float fovAngle_ = 60.0f * Math::ToRadian;
	// ライトの射程
	float lightRange_ = 10.0f;

	
	//ライト自信のTransform
	Transform transform_;
	//ワールド座標での注視点
	Vector3 focusPoint_;
	float focusDistance_;
	//親からの「ローカルオフセット」
	Vector3 flashLightOffset_;
	float moveSpeed_;
	//し錐台
	float fovAngleRadians_;
	float range_;
	float focusT_;
};