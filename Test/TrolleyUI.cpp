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
	batteryModel_.SetModel(assetManager->modelMap.Get("batteryModel")->Get());

	auto baseTexture = assetManager->textureMap.Get("TrolleyBase")->Get();
	auto chargeGaugeTexture = assetManager->textureMap.Get("TrolleyChargeGauge")->Get();
	auto overChargeGaugeTexture = assetManager->textureMap.Get("TrolleyOverChargeGauge")->Get();
	auto nitroGaugeTexture = assetManager->textureMap.Get("TrolleyNitroGauge")->Get();
	auto nitroBurstTexture = assetManager->textureMap.Get("TrolleyNitroBurst")->Get();

	baseUI_.SetTexture(baseTexture);
	chargeUI_.SetTexture(chargeGaugeTexture);
	overChargeUI_.SetTexture(overChargeGaugeTexture);
	nitroBurstUI_.SetTexture(nitroBurstTexture);
	nitroUI_.SetTexture(nitroGaugeTexture);


	baseUI_.SetPosition({ 1064.0f,77.0f });
	baseUI_.SetScale(baseTexture->GetSize());
	baseUI_.SetAnchor({ 0.5f, 0.5f });
	baseUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	baseUI_.SetDrawOrder(0);

	chargeUI_.SetPosition({ 972.0f,77.0f });
	chargeUI_.SetScale(chargeGaugeTexture->GetSize());
	chargeUI_.SetAnchor({ 0.0f,0.5f });
	chargeUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	chargeUI_.SetDrawOrder(1);

	overChargeUI_.SetPosition({ 972.0f,77.0f });
	overChargeUI_.SetScale(overChargeGaugeTexture->GetSize());
	overChargeUI_.SetAnchor({ 0.0f,0.5f });
	overChargeUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	overChargeUI_.SetDrawOrder(2);

	nitroBurstUI_.SetPosition({ 972.0f,77.0f });
	nitroBurstUI_.SetScale(nitroBurstTexture->GetSize());
	nitroBurstUI_.SetAnchor({ 0.0f,0.5f });
	nitroBurstUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	nitroBurstUI_.SetIsActive(false);
	nitroBurstUI_.SetDrawOrder(3);


	nitroUI_.SetPosition({ 1081.0f,61.0f });
	nitroUI_.SetScale(nitroGaugeTexture->GetSize());
	nitroUI_.SetAnchor({ 0.0f,0.5f });
	nitroUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
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

	batteryModel_.SetWorldMatrix(trolley_->GetBatteyTransform(0).worldMatrix);


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
	
	batteryModel_.SetWorldMatrix(trolley_->GetBatteyTransform(0).worldMatrix);

	speedMeterModel_.SetWorldMatrix(speedMeterTransform_.worldMatrix);
	speedMeterNeedleModel_.SetWorldMatrix(speedMeterNeedleTransform_.worldMatrix);

	float currentCharge = trolley_->GetCurrentCharge();
	float maxNormalChargeTime = trolley_->GetMaxNormalCharge();
	float burstThreshold = trolley_->GetBurstThreshold();

	float chargeT = std::clamp(trolley_->GetCurrentCharge() / trolley_->GetMaxNormalCharge(), 0.0f, 1.0f);
	float nitroT = std::clamp(trolley_->GetNitroAccumulateTimer() / trolley_->GetNitroChargeTime(), 0.0f, 1.0f);
	float overChargeT = std::clamp((currentCharge - maxNormalChargeTime) / (burstThreshold - maxNormalChargeTime), 0.0f, 1.0f);

	chargeUI_.SetScale({ std::lerp(0.0f,268.0f,chargeT),44.0f });
	nitroUI_.SetScale({ std::lerp(0.0f,144.0f,nitroT),12.0f });
	overChargeUI_.SetScale({ std::lerp(0.0f,268.0f,overChargeT),44.0f  });

	chargeUI_.SetUVRect({ {0.0f,0.0f} ,{chargeT,1.0f} }, Sprite::UVMode::UV);
	nitroUI_.SetUVRect({ {0.0f,0.0f} ,{nitroT,1.0f} }, Sprite::UVMode::UV);
	overChargeUI_.SetUVRect({ {0.0f,0.0f} ,{overChargeT,1.0f} }, Sprite::UVMode::UV);


	if (trolley_->GetState() == Trolley::State::Nitro) {
		nitroBurstUI_.SetIsActive(true);
	}
	else {
		nitroBurstUI_.SetIsActive(false);
	}
#ifdef _DEBUG
	DrawImGui();
	baseUI_.DrawImGui("TrolleyBaseUI");
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

void TrolleyUI::UpdateSprite(Sprite& sprite, float t, float size)
{
	sprite, t, size;
	//sprite.SetScale();

}

#endif // _DEBUG