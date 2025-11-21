#pragma once

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Camera.h"
#include "Graphics/Model.h"

class Flashlight {
public:
	void Initialize(const Transform* parentTransform, const Camera* parentCamera);
	void Update();

private:
    void SpotLightDebugDraw() const;
	void DebugMove();

	const Transform* parentTransform_ = nullptr;
	const Camera* parentCamera_ = nullptr;
	
	ModelInstance lightModel_;
    Transform lightTransform_;
	//ライト自信のTransform
	Transform transform_;
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
	
};