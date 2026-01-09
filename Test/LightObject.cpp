#include "LightObject.h"
#include "Test/Trolley.h"

void LightObject::Initialize(const Transform* parentTransform, const Vector3& offset, bool isBreath) {
	offset_ = offset;
	light_ = std::make_shared<PointLight>();
	if (parentTransform != nullptr) {
		parentTransform_ = parentTransform;
		lightTransform_.SetParent(parentTransform_, false);
	}
	lightTransform_.translate = offset_;
	lightTransform_.UpdateMatrix();
	light_->position = lightTransform_.worldMatrix.GetTranslate();
	isBreath_ = isBreath;
	RenderManager::GetInstance()->GetLightManager().Add(light_);
}

void LightObject::Update() {
	frame_++;
	lightTransform_.translate = offset_;
	lightTransform_.UpdateMatrix();
	light_->position = lightTransform_.worldMatrix.GetTranslate();

	healthStatus_.Update();

	float currentIntensity = Math::Lerp(healthStatus_.hp, saveIntensity_ * 0.1f, saveIntensity_);
	float currentRange = Math::Lerp(healthStatus_.hp, saveRange_ * 0.1f, saveRange_);

	if (isBreath_ && healthStatus_.hp > 0.0f) {
		// 0.05f だと 60fpsでおよそ2秒で1周
		constexpr float kBreathSpeed = 0.05f;
		// 振幅
		float kIntensityAmplitude = 1.0f * lightTransform_.worldMatrix.GetScale().x;
		float kRangeAmplitude = 0.2f * lightTransform_.worldMatrix.GetScale().x;

		currentIntensity += std::sin(static_cast<float>(frame_) * kBreathSpeed) * kIntensityAmplitude;
		currentRange += std::sin(static_cast<float>(frame_) * kBreathSpeed) * kRangeAmplitude;
	}

	light_->intensity = currentIntensity;
	light_->range = currentRange;

	if (Vector3::Distance(Trolley::GetInstance()->GetTransform().worldMatrix.GetTranslate(), lightTransform_.worldMatrix.GetTranslate()) <= lightActiveDistance) {
		isActive_ = true;
	}
	else {
		isActive_ = false;
	}

}


#ifdef _DEBUG
void LightObject::Debug(const std::string& label)
{
	(void)label;
#ifdef ENABLE_IMGUI
	bool openTree = ImGui::TreeNode(label.c_str());
	if (openTree) {
		ImGui::DragFloat3("offset", &offset_.x, 0.01f);
		light_->DrawImGui(label + "lightSetting");
		ImGui::TreePop();
	}
#endif //ENABLE_IMGUI

}
#endif // _DEBUG

void LightObject::HealthStatus::Update()
{
	if (!isTakingDamage) { return; }
	damageTimer += 1.0f;
	hp = std::lerp(1.0f, 0.0f, std::clamp((damageTimer / damageDuration), 0.0f, 1.0f));
}
