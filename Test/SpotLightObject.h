#pragma once

#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Graphics/LightManager.h"

#include "Collider.h"

class SpotLightObject {
public:
	void Initialize(const Transform* parentTransform, const Vector3& offset = { 0.0f,0.0f,0.0f }, const Vector3& direction = {0.0f,0.0f,0.0f});
	void Update();
#ifdef _DEBUG
	void Debug(const std::string& label);
#endif // _DEBUG
	void SetOffset(const Vector3& offset) { offset_ = offset; }
	void SetDirection(const Vector3& direction) { direction_ = direction; }
	void SetLightSetting(const SpotLight& spotLightSetting) {
		light_->color = spotLightSetting.color;
		light_->intensity = spotLightSetting.intensity;
		light_->range = spotLightSetting.range;
		light_->decay = spotLightSetting.decay;
		light_->angle = spotLightSetting.angle;
		light_->direction = spotLightSetting.direction;
	};
private:

	const Transform* parentTransform_ = nullptr;
	Transform lightTransform_;
	std::shared_ptr<SpotLight> light_;
	Vector3 offset_;
	Vector3 direction_;

#ifdef _DEBUG
	bool isDebug_ = true;
#endif // _DEBUG
};