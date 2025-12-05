#pragma once

#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Graphics/LightManager.h"

#include "Collider.h"

static const float deadDecayParam = 10.0f;

class LightObject {
public:
	void Initialize(const Transform* parentTransform, const Vector3& offset = {0.0f,0.0f,0.0f},bool isBreath);
	void Update();
#ifdef _DEBUG
	void Debug(const std::string& label);
#endif // _DEBUG
	void SetHp(float hp) {
		hp_ = hp;
	}
	void SetOffset(const Vector3& offset) { offset_ = offset; }
	void SetLightSetting(const PointLight& pointLightSetting) {
		light_->color = pointLightSetting.color;
		light_->intensity = pointLightSetting.intensity;
		light_->range = pointLightSetting.range;
		if (!firstDecaySet_) {
			firstDecaySet_ = true;
			saveDecay_ = pointLightSetting.decay;
			light_->decay = pointLightSetting.decay;
		}
	};
private:

	const Transform* parentTransform_ = nullptr;
	Transform lightTransform_;
	std::shared_ptr<PointLight> light_;
	Vector3 offset_;
	bool firstDecaySet_ = false;
	float saveDecay_ = 1.0f;

	float hp_ = 1.0f;
	bool isBreath_ = false;
	uint32_t frame_ = 0;

#ifdef _DEBUG
	bool isDebug_ = true;
#endif // _DEBUG
};