#include "LightObject.h"

void LightObject::Initialize(const Transform* parentTransform, const Vector3& offset) {
	offset_ = offset;
	light_ = std::make_shared<PointLight>();
	if (parentTransform != nullptr) {
		parentTransform_ = parentTransform;
		lightTransform_.SetParent(parentTransform_,false);
	}
	lightTransform_.translate = offset_;
	lightTransform_.UpdateMatrix();
	light_->position = lightTransform_.worldMatrix.GetTranslate();
	RenderManager::GetInstance()->GetLightManager().Add(light_);
}

void LightObject::Update() {
	lightTransform_.translate = offset_;
	lightTransform_.UpdateMatrix();
	light_->position = lightTransform_.worldMatrix.GetTranslate();
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