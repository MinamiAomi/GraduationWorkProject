#pragma once

#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Graphics/LightManager.h"

#include "Collider.h"

static const float lightActiveDistance = 100.0f;

class LightObject {
public:
	void Initialize(const Transform* parentTransform, const Vector3& offset = { 0.0f,0.0f,0.0f }, bool isBreath = true);
	void Update();
#ifdef _DEBUG
	void Debug(const std::string& label);
#endif // _DEBUG
	bool GetIsActive() const { return isActive_; }




	std::shared_ptr<const Model> GetModelResource() const {
		return model_;
	}

	void SetModel(std::shared_ptr<Model> model) {
		model_ = model;
	}

	const Transform* GetTransform() const {
		return &lightTransform_;
	}

	void SetDamage(float damage) { damage_ = damage; }
	void SetMaxHp(float maxHp) { maxHp_ = maxHp; }
	float GetDamage() const { return damage_; }
	void SetHp(float hp) { hp_ = hp; }
	float GetHp() const { return hp_; }
	float GetMaxHp() const { return maxHp_; }
	bool GetIsAlive() const { return isAlive_; }
	const Color& GetColor() const{ return light_->color; }
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
	void HpUpdate();
	const Transform* parentTransform_ = nullptr;
	std::shared_ptr<Model> model_;
	Transform lightTransform_;
	std::shared_ptr<PointLight> light_;
	Vector3 offset_;
	bool firstSet_ = false;
	float saveIntensity_ = 1.0f;
	float saveRange_ = 1.0f;

	bool isBreath_ = false;
	bool isActive_ = false;
	uint32_t frame_ = 0;

	bool isAlive_;

#pragma region HP関係
	float damage_;
	float maxHp_;
	float hp_;
#pragma endregion


#ifdef _DEBUG
	bool isDebug_ = true;
#endif // _DEBUG
};