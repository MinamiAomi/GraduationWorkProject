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
	JSON_OBJECT("TrollerSpeed");
	JSON_LOAD(speedMeterOffset_);
	JSON_LOAD(speedMeterNeedleOffset_);
	JSON_ROOT();
	JSON_CLOSE();


	speedMeterTransform_.translate = speedMeterOffset_;
	speedMeterNeedleTransform_.translate = speedMeterNeedleOffset_;

	speedMeterTransform_.SetParent(&transform);
	speedMeterNeedleTransform_.SetParent(&transform);

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

	speedMeterNeedleTransform_.rotate = Quaternion::Slerp(trolley_->GetTrollySpeedRatio(),Quaternion::MakeForZAxis(270.0f), Quaternion::MakeForZAxis(-90.0f)) ;

	speedMeterTransform_.translate = speedMeterOffset_;
	speedMeterNeedleTransform_.translate = speedMeterNeedleOffset_;

	speedMeterTransform_.UpdateMatrix();
	speedMeterNeedleTransform_.UpdateMatrix();

	speedMeterModel_.SetWorldMatrix(speedMeterTransform_.worldMatrix);
	speedMeterNeedleModel_.SetWorldMatrix(speedMeterNeedleTransform_.worldMatrix);

#ifdef _DEBUG
	ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::TreeNode("Troller")) {
		if (ImGui::TreeNode("TrollerUI")) {
			ImGui::DragFloat3("SpeedMeterOffset", &speedMeterOffset_.x, 0.1f);
			ImGui::DragFloat3("NeedleOffset", &speedMeterNeedleOffset_.x, 0.1f);

			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Trolley/trolleyUI.json");
				JSON_OBJECT("TrollerUI");
				JSON_SAVE(speedMeterOffset_);
				JSON_SAVE(speedMeterNeedleOffset_);
				JSON_ROOT();
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	ImGui::End();
#endif // _DEBUG

}
