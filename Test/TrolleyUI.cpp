#include "TrolleyUI.h"

#include "Framework/AssetManager.h"
#include "Engine/File/JsonConverter.h"

#include "Trolley.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG


TrolleyUI::TrolleyUI()
{
	auto assetManager = AssetManager::GetInstance();
	speedMeterModel_.SetModel(assetManager->modelMap.Get("speedMeter")->Get());
	speedMeterNeedleModel_.SetModel(assetManager->modelMap.Get("speedMeterNeedle")->Get());

	chargeUI_.SetTexture(AssetManager::GetInstance()->textureMap.Get("white2x2")->Get());
	overChargeUI_.SetTexture(AssetManager::GetInstance()->textureMap.Get("white2x2")->Get());
	nitroUI_.SetTexture(AssetManager::GetInstance()->textureMap.Get("white2x2")->Get());
}

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

	
	speedMeterModel_.SetWorldMatrix(speedMeterTransform_.worldMatrix);
	speedMeterNeedleModel_.SetWorldMatrix(speedMeterNeedleTransform_.worldMatrix);

	

	chargeUI_.SetPosition({ 915.0f,50.0f });
	chargeUI_.SetScale({ 300.0f,30.0f });
	chargeUI_.SetAnchor({ 0.0f,0.5f });
	chargeUI_.SetColor({ 0.2f,0.2f,0.7f,1.0f });
	chargeUI_.SetDrawOrder(0);

	overChargeUI_.SetPosition({ 915.0f,50.0f });
	overChargeUI_.SetScale({ 300.0f,30.0f });
	overChargeUI_.SetAnchor({ 0.0f,0.5f });
	overChargeUI_.SetColor({ 0.7f,0.2f,0.2f,1.0f });
	overChargeUI_.SetDrawOrder(1);

	nitroBurstUI_.SetPosition({ 915.0f,50.0f });
	nitroBurstUI_.SetScale({ 300.0f,30.0f });
	nitroBurstUI_.SetAnchor({ 0.0f,0.5f });
	nitroBurstUI_.SetColor({ 0.7f,0.7f,0.2f,1.0f });
	nitroBurstUI_.SetIsActive(false);
	nitroBurstUI_.SetDrawOrder(2);


	nitroUI_.SetPosition({ 915.0f,77.0f });
	nitroUI_.SetScale({ 300.0f,5.0f });
	nitroUI_.SetAnchor({ 0.0f,0.5f });
	nitroUI_.SetColor({ 0.7f,0.7f,0.2f,1.0f });
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

	float currentCharge = trolley_->GetCurrentCharge();
	float maxNormalChargeTime = trolley_->GetMaxNormalCharge();
	float burstThreshold = trolley_->GetBurstThreshold();

	float chargeT = std::clamp(trolley_->GetCurrentCharge() / trolley_->GetMaxNormalCharge(), 0.0f, 1.0f);
	float nitroT = std::clamp(trolley_->GetNitroAccumulateTimer() / trolley_->GetNitroChargeTime(), 0.0f, 1.0f);
	float overChargeT = std::clamp((currentCharge - maxNormalChargeTime) / (burstThreshold - maxNormalChargeTime), 0.0f, 1.0f);

	chargeUI_.SetScale({ std::lerp(0.0f,300.0f,chargeT),30.0f });
	nitroUI_.SetScale({ std::lerp(0.0f,300.0f,nitroT),5.0f });
	overChargeUI_.SetScale({ std::lerp(0.0f,300.0f,overChargeT),30.0f });
	if (trolley_->GetState()==Trolley::State::Nitro) {
		nitroBurstUI_.SetIsActive(true);
	}
	else {
		nitroBurstUI_.SetIsActive(false);
	}
#ifdef _DEBUG
	DrawImGui();
	chargeUI_.DrawImGui("ChargeUI");
	overChargeUI_.DrawImGui("OverChargeUI");
	nitroUI_.DrawImGui("NitroUI");
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
		ImGui::DragFloat3("スピードメーターの針オフセット", &speedMeterNeedleOffset_.x, 0.01f);


		ImGui::TreePop();
	}
	ImGui::End();
}

#endif // _DEBUG