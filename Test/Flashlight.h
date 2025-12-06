#pragma once

#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Camera.h"
#include "Graphics/Model.h"
#include "Graphics/LightManager.h"

#include "Collider.h"

class Flashlight {
public:
	void Initialize(const Transform* parentTransform, const Camera* parentCamera);
	void Update();

	std::shared_ptr<ConeCollider> GetCollider() { return collider_; }

	bool GetIsLighting() const { return isLighting_; }
private:
	void UpdateCollision();
	void UpdateLightPower();
	void SpotLightDebugDraw() const;
	void DebugMove();
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG


	const Transform* parentTransform_ = nullptr;
	const Camera* parentCamera_ = nullptr;

	std::shared_ptr<SpotLight> spotLight_;
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
	float fovAngle_ = 15.0f * Math::ToRadian;
	// ライトの射程
	float lightRange_ = 40.0f;

#pragma region ライトの電池残量関連
	//現在の残量
	float battery_;
	//最大値
	float maxBattery_;
	//加算地
	float addBattery_;
	//減算地
	float subBattery_;
	//照らしているか
	bool isLighting_;
#pragma endregion

	std::shared_ptr<ConeCollider> collider_;
#ifdef _DEBUG
	bool isDebug_ = true;
#endif // _DEBUG
};