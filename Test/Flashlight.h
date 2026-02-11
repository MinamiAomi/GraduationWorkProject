#pragma once

#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Camera.h"
#include "Graphics/Model.h"
#include "Graphics/LightManager.h"

#include "FlashlightUI.h"
#include "Collider.h"
#include "Math/Random.h"

namespace RailSystem {
	class RailAnimationPlayer;
}

class Flashlight {
public:
	Flashlight();
	void Initialize(const Transform* parentTransform, const Camera* parentCamera);
	void Update();

	const std::shared_ptr<ConeCollider> GetCollider() const { return collider_; }
	float GetBattery() const { return battery_; }
	float GetMaxBattery() const { return maxBattery_; }
	bool GetIsLighting() const { return isLighting_; }
	void SetRailAnimationPlayer(const RailSystem::RailAnimationPlayer* railAnimationPlayer) { railAnimationPlayer_ = railAnimationPlayer; }
	const Transform& GetTransform() const { return transform_; }
	void Pause() { isPause_ = true; }
	void Play() { isPause_ = false; }

	bool GetBatteryRemaining() const { return  (battery_ < maxBattery_ * 0.2f); }
private:
	void UpdateCollision();
	void UpdateLightPower();
	void SpotLightDebugDraw() const;
	void Move();
	void DebugMove();
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG

	Random::RandomNumberGenerator rnd_;
	const Transform* parentTransform_ = nullptr;
	const Camera* parentCamera_ = nullptr;

	const RailSystem::RailAnimationPlayer* railAnimationPlayer_;

	std::shared_ptr<SpotLight> spotLight_;
	ModelInstance lightModel_;
	//FlashlightUI flashlightUI_;

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
	//ライトが当たっているか
	bool isHitFlashlight_;
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
	//何フレーム目まで減らないか
	float startFrame_;

	float blinkTimer_ = 0.0f;
#pragma endregion
	std::shared_ptr<ConeCollider> collider_;
	bool isPause_ = false;
};