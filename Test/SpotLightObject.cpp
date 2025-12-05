#include "SpotLightObject.h"

void SpotLightObject::Initialize(const Transform* parentTransform, const Vector3& offset, const Vector3& direction = { 0.0f,0.0f,0.0f }) {
	offset_ = offset;
	direction_ = direction;
	light_ = std::make_shared<SpotLight>();
	if (parentTransform != nullptr) {
		parentTransform_ = parentTransform;
		lightTransform_.SetParent(parentTransform_,false);
	}
	lightTransform_.translate = offset_;
	lightTransform_.UpdateMatrix();
	light_->position = lightTransform_.worldMatrix.GetTranslate();
	light_->direction = lightTransform_.worldMatrix.ApplyRotation(direction_);
	RenderManager::GetInstance()->GetLightManager().Add(light_);
}

void SpotLightObject::Update() {
	lightTransform_.translate = offset_;
	lightTransform_.UpdateMatrix();
	light_->direction = lightTransform_.worldMatrix.ApplyRotation(direction_);
	light_->position = lightTransform_.worldMatrix.GetTranslate();
}


#ifdef _DEBUG
void SpotLightObject::Debug(const std::string& label)
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