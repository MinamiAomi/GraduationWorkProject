#include "LightObject.h"

void LightObject::Initialize(const Transform* parentTransform, const Vector3& offset, bool isBreath) {
	offset_ = offset;
	light_ = std::make_shared<PointLight>();
	if (parentTransform != nullptr) {
		parentTransform_ = parentTransform;
		lightTransform_.SetParent(parentTransform_,false);
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

	float currentDecay = Math::Lerp(hp_, deadDecayParam, saveDecay_);

	if (isBreath_ && hp_ > 0.0f) {
		// 0.05f だと 60fpsでおよそ2秒で1周
		constexpr float kBreathSpeed = 0.05f;
		// 振幅
		constexpr float kBreathAmplitude = 2.0f;
		float breathValue = std::sin(static_cast<float>(frame_) * kBreathSpeed) * kBreathAmplitude;
		currentDecay += breathValue;
	}
	light_->decay = Math::Lerp(hp_,deadDecayParam , saveDecay_);

}


#ifdef _DEBUG
void LightObject::Debug(const std::string& label)
{
	(void)label;
#ifdef ENABLE_IMGUI
	bool openTree = ImGui::TreeNode(label.c_str());
	if (openTree) {
		ImGui::DragFloat3( "offset", &offset_.x, 0.01f);
		light_->DrawImGui(label + "lightSetting");
		ImGui::TreePop();
	}
#endif //ENABLE_IMGUI
	
}
#endif // _DEBUG