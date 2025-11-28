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
	JSON_LOAD(needleOffset_);
	JSON_ROOT();
	JSON_CLOSE();


	speedMeterTransform_.translate = speedMeterOffset_;
	needleTransform_.translate = needleOffset_;

	speedMeterTransform_.SetParent(&transform);
	needleTransform_.SetParent(&transform);

	speedMeterTransform_.UpdateMatrix();
	needleTransform_.UpdateMatrix();

	auto assetManager = AssetManager::GetInstance();
	speedMeterModel_.SetModel(assetManager->modelMap.Get("speedMeter")->Get());
	needleModel_.SetModel(assetManager->modelMap.Get("needle")->Get());
	speedMeterModel_.SetWorldMatrix(speedMeterTransform_.worldMatrix);
	needleModel_.SetWorldMatrix(needleTransform_.worldMatrix);
}

void TrolleyUI::Update()
{

	needleTransform_.rotate = Quaternion::Slerp(trolley_->GetTrollySpeedRatio(),Quaternion::MakeForZAxis(180.0f), Quaternion::MakeForZAxis(0.0f)) ;

	speedMeterTransform_.translate = speedMeterOffset_;
	needleTransform_.translate = needleOffset_;

	speedMeterTransform_.UpdateMatrix();
	needleTransform_.UpdateMatrix();

#ifdef _DEBUG
	ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::TreeNode("Troller")) {
		if (ImGui::TreeNode("TrollerUI")) {
			ImGui::DragFloat3("SpeedMeterOffset", &speedMeterOffset_.x, 0.1f);
			ImGui::DragFloat3("NeedleOffset", &needleOffset_.x, 0.1f);

			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Trolley/trolleyUI.json");
				JSON_OBJECT("TrollerUI");
				JSON_SAVE(speedMeterOffset_);
				JSON_SAVE(needleOffset_);
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
