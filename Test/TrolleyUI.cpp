#include "TrolleyUI.h"

#include "Framework/AssetManager.h"
#include "Engine/File/JsonConverter.h"

#include "Trolley.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG


void TrolleyUI::Initialize(const Transform& transform)
{
	JSON_OPEN("Resources/Data/Trolley/trolleyUI.json");
	JSON_OBJECT("TrollerUI");
	JSON_LOAD(speedMeterOffset_);
	JSON_LOAD(speedMeterNeedleOffset_);
	JSON_ROOT();
	JSON_CLOSE();


	speedMeterTransform_.translate = speedMeterOffset_;
	speedMeterNeedleTransform_.translate = speedMeterNeedleOffset_;

	speedMeterTransform_.SetParent(&transform);
	speedMeterNeedleTransform_.SetParent(&speedMeterTransform_);

	speedMeterTransform_.UpdateMatrix();
	speedMeterNeedleTransform_.UpdateMatrix();

	auto assetManager = AssetManager::GetInstance();
	speedMeterModel_.SetModel(assetManager->modelMap.Get("speedMeter")->Get());
	speedMeterNeedleModel_.SetModel(assetManager->modelMap.Get("speedMeterNeedle")->Get());
	speedMeterModel_.SetWorldMatrix(speedMeterTransform_.worldMatrix);
	speedMeterNeedleModel_.SetWorldMatrix(speedMeterNeedleTransform_.worldMatrix);
}

void TrolleyUI::Update()
{
	speedMeterNeedleTransform_.rotate = Quaternion::Slerp(
		trolley_->GetTrollySpeedRatio(),
		Quaternion::MakeForZAxis(90.0f * Math::ToRadian),
		Quaternion::MakeForZAxis(-90.0f * Math::ToRadian)
	);
	speedMeterTransform_.translate = speedMeterOffset_;
	speedMeterNeedleTransform_.translate = speedMeterNeedleOffset_;

	speedMeterTransform_.UpdateMatrix();
	speedMeterNeedleTransform_.UpdateMatrix();

	speedMeterModel_.SetWorldMatrix(speedMeterTransform_.worldMatrix);
	speedMeterNeedleModel_.SetWorldMatrix(speedMeterNeedleTransform_.worldMatrix);

#ifdef _DEBUG
	DrawImGui();
#endif // _DEBUG

}

#ifdef _DEBUG
void TrolleyUI::DrawImGui()
{
	ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::TreeNode("トロッコUI (TrollerUI)")) {
		if (ImGui::Button("Save", ImVec2(-1.0f, 0.0f))) {
			JSON_OPEN("Resources/Data/Trolley/trolleyUI.json");
			JSON_OBJECT("TrollerUI");
			JSON_SAVE(speedMeterOffset_);
			JSON_SAVE(speedMeterNeedleOffset_);
			JSON_ROOT();
			JSON_CLOSE();
		}
		ImGui::Separator();

		ImGui::DragFloat3("スピードメーターオフセット", &speedMeterOffset_.x, 0.01f);
		ImGui::DragFloat3("はぴーどメーターの針オフセット", &speedMeterNeedleOffset_.x, 0.01f);


		ImGui::TreePop();
	}
	ImGui::End();
}

#endif // _DEBUG