#pragma once

#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Graphics/LightManager.h"

#include "Collider.h"

static const float lightActiveDistance = 200.0f;

class LightObject {
public:
	void Initialize(const Transform* parentTransform, const Vector3& offset = { 0.0f,0.0f,0.0f }, bool isBreath = true);
	void Update();
#ifdef _DEBUG
	void Debug(const std::string& label);
#endif // _DEBUG
	void SetHp(float hp) {
		healthStatus_.hp = hp;
	}
	float GetHp() { return healthStatus_.hp;}

	//何秒かけて死ぬか
	void SetDamageDuration(float maxHpFrame) {
		healthStatus_.isTakingDamage = true;
		healthStatus_.damageDuration= maxHpFrame;
	}
	void SetOffset(const Vector3& offset) { offset_ = offset; }
	void SetLightSetting(const PointLight& pointLightSetting) {
		light_->color = pointLightSetting.color;
		light_->decay = pointLightSetting.decay;
		if (!firstSet_) {
			firstSet_ = true;
			saveIntensity_ = pointLightSetting.intensity;
			saveRange_ = pointLightSetting.range;
			light_->intensity = pointLightSetting.intensity;
			light_->range = pointLightSetting.range;
		}
	};
private:

	const Transform* parentTransform_ = nullptr;
	Transform lightTransform_;
	std::shared_ptr<PointLight> light_;
	Vector3 offset_;
	bool firstSet_ = false;
	float saveIntensity_ = 1.0f;
	float saveRange_ = 1.0f;

	bool isBreath_ = false;
	bool isActive_ = false;
	uint32_t frame_ = 0;

#pragma region HP関係
	struct HealthStatus {
		bool isTakingDamage = false;
		float damageTimer = 0.0f;
		float damageDuration = 0.0f;
		//旧HP
		float hp = 1.0f;

		void Update();
	}healthStatus_;

#pragma endregion


#ifdef _DEBUG
	bool isDebug_ = true;
#endif // _DEBUG
};