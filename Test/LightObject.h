#pragma once

#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Graphics/LightManager.h"

#include "Collider.h"

class LightObject {
public:
	void Initialize(const Transform* parentTransform, const Vector3& offset = {0.0f,0.0f,0.0f});
	void Update();
#ifdef _DEBUG
	void Debug(const std::string& label);
#endif // _DEBUG
	void SetOffset(const Vector3& offset) { offset_ = offset; }
	void SetLightSetting(const PointLight& pointLightSetting) {
		light_->color = pointLightSetting.color;
		light_->intensity = pointLightSetting.intensity;
		light_->range = pointLightSetting.range;
		light_->decay = pointLightSetting.decay;
	};
private:

	const Transform* parentTransform_ = nullptr;
	Transform lightTransform_;
	std::shared_ptr<PointLight> light_;
	Vector3 offset_;

#ifdef _DEBUG
	bool isDebug_ = true;
#endif // _DEBUG
};