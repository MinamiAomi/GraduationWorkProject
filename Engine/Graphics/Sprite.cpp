#include "Sprite.h"

#include "Core/TextureLoader.h"
#include "Core/SamplerManager.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG


std::list<Sprite*> Sprite::instanceList_;

Sprite::Sprite() {
	instanceList_.emplace_back(this);
}

Sprite::~Sprite() {
	instanceList_.remove(this);
}

#ifdef _DEBUG
void Sprite::DrawImGui(const std::string& name) {
	ImGui::PushID(this);
	if (ImGui::TreeNode(name.c_str())) {



		ImGui::TextDisabled("Transform");
		ImGui::DragFloat2("Center", &position_.x, 0.1f);
		ImGui::DragFloat("Rotate", &rotate_, 0.1f);
		ImGui::DragFloat2("Scale", &scale_.x, 0.01f);
		ImGui::DragFloat2("Anchor", &anchor_.x, 0.01f);

		ImGui::Separator();

		ImGui::TextDisabled("Texture & UV");
		if (ImGui::TreeNode("UV Rect Settings")) {
			ImGui::DragFloat2("Base", &texcoordBase_.x, 0.01f);
			ImGui::DragFloat2("Size", &texcoordSize_.x, 0.01f);
			ImGui::TreePop();
		}

		ImGui::Separator();

		ImGui::TextDisabled("Rendering");

		ImGui::ColorEdit4("Color", &color_.x);

		int order = static_cast<int>(drawOrder_);
		if (ImGui::DragInt("Draw Order", &order)) {
			drawOrder_ = static_cast<uint8_t>(order);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();
}
#endif // _DEBUG