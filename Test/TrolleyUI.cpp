#include "TrolleyUI.h"

#include "Framework/AssetManager.h"
#include "Engine/File/JsonConverter.h"

#include "Trolley.h"
#include "Scene/SceneManager.h"

#include "GameScene.h"
#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG


TrolleyUI::TrolleyUI()
{
	auto assetManager = AssetManager::GetInstance();
	speedMeterModel_.SetModel(assetManager->modelMap.Get("speedMeter")->Get());
	speedMeterNeedleModel_.SetModel(assetManager->modelMap.Get("speedMeterNeedle")->Get());

	insideModel_.SetModel(assetManager->modelMap.Get("batteryInside")->Get());
	for (auto& original : insideModel_.GetModel()->GetMaterials()) {
		auto matInstance = std::make_shared<Material>();
		*matInstance = original;
		insideModel_.GetMaterials().push_back(matInstance);
	}

	outsideModel_.SetModel(assetManager->modelMap.Get("batteryOutside")->Get());
	for (auto& original : outsideModel_.GetModel()->GetMaterials()) {
		auto matInstance = std::make_shared<Material>();
		*matInstance = original;
		outsideModel_.GetMaterials().push_back(matInstance);
	}
	effectModel_.SetModel(assetManager->modelMap.Get("batteryEffect")->Get());
	for (auto& original : effectModel_.GetModel()->GetMaterials()) {
		auto matInstance = std::make_shared<Material>();
		*matInstance = original;
		effectModel_.GetMaterials().push_back(matInstance);
	}
	//batteryModel_.SetModel(assetManager->modelMap.Get("batteryModel")->Get());

	auto baseTexture = assetManager->textureMap.Get("TrolleyBase")->Get();
	auto chargeGaugeTexture = assetManager->textureMap.Get("TrolleyChargeGauge")->Get();
	auto nitroGaugeTexture = assetManager->textureMap.Get("TrolleyNitroGauge")->Get();
	auto nitroBurstTexture = assetManager->textureMap.Get("TrolleyNitroBurst")->Get();


	baseUI_.SetTexture(baseTexture);
	chargeUI_.SetTexture(chargeGaugeTexture);
	nitroBurstUI_.SetTexture(nitroBurstTexture);
	nitroUI_.SetTexture(nitroGaugeTexture);

	baseUI_.SetPosition({ 1120.0f,60.0f });
	baseUI_.SetScale(baseTexture->GetSize());
	baseUI_.SetAnchor({ 0.5f, 0.0f });
	baseUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	baseUI_.SetDrawOrder(3);
	baseUI_.SetIsActive(false);

	chargeUISize_ = chargeGaugeTexture->GetSize();
	chargeUI_.SetPosition({ 1134.0f,60.0f});
	chargeUI_.SetScale(chargeUISize_);
	chargeUI_.SetAnchor({ 0.5f,0.0f });
	chargeUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	chargeUI_.SetDrawOrder(1);

	nitroBurstUISize_ = nitroBurstTexture->GetSize();
	nitroBurstUI_.SetPosition({ 1134.0f,60.0f });
	nitroBurstUI_.SetScale(nitroBurstUISize_);
	nitroBurstUI_.SetAnchor({ 0.5f,0.0f });
	nitroBurstUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	nitroBurstUI_.SetIsActive(false);
	nitroBurstUI_.SetDrawOrder(2);


	nitroUISize_ = nitroGaugeTexture->GetSize();
	nitroUI_.SetPosition({ 1090.0f,66.0f });
	nitroUI_.SetScale(nitroUISize_);
	nitroUI_.SetAnchor({ 0.5f,0.0f });
	nitroUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);

	if (dynamic_cast<GameScene*>(SceneManager::GetInstance()->GetCurrentScene())) {

		//model
		speedMeterModel_.SetIsActive(true);
		speedMeterNeedleModel_.SetIsActive(true);
		effectModel_.SetIsActive(true);
		insideModel_.SetIsActive(true);
		outsideModel_.SetIsActive(true);
		batteryModel_.SetIsActive(true);

		//sprite
		chargeUI_.SetIsActive(true);
		baseUI_.SetIsActive(true);
		nitroUI_.SetIsActive(true);
		nitroBurstUI_.SetIsActive(true);
		baseUI_.SetIsActive(true);
	}
	else {
		//model
		speedMeterModel_.SetIsActive(false);
		speedMeterNeedleModel_.SetIsActive(false);
		effectModel_.SetIsActive(false);
		insideModel_.SetIsActive(false);
		outsideModel_.SetIsActive(false);
		batteryModel_.SetIsActive(false);
		//sprite
		chargeUI_.SetIsActive(false);
		baseUI_.SetIsActive(false);
		nitroUI_.SetIsActive(false);
		nitroBurstUI_.SetIsActive(false);
		baseUI_.SetIsActive(false);
	}
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

	batterTransform_.SetParent(&trolley_->GetBatteyTransform(0), false);

	speedMeterTransform_.UpdateMatrix();
	speedMeterNeedleTransform_.UpdateMatrix();
	batterTransform_.UpdateMatrix();

	insideTransform_.SetParent(&batterTransform_, false);
	outsideTransform_.SetParent(&batterTransform_, false);
	effectTransform_.SetParent(&batterTransform_, false);

	insideTransform_.UpdateMatrix();
	outsideTransform_.UpdateMatrix();
	effectTransform_.UpdateMatrix();

	effectModel_.SetWorldMatrix(effectTransform_.worldMatrix);
	insideModel_.SetWorldMatrix(insideTransform_.worldMatrix);
	outsideModel_.SetWorldMatrix(outsideTransform_.worldMatrix);


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

	if (trolley_->GetIsHitFlashlight() && trolley_->GetBatteryRemaining()) {
		float rate = trolley_->GetCenterRate();

		float baseSpeed = rate * 0.1f;


		insideTransform_.rotate *= Quaternion::MakeFromAngleAxis(baseSpeed * 1.2f, insideAxis_);

		outsideTransform_.rotate *= Quaternion::MakeFromAngleAxis(baseSpeed * 0.9f, outsideAxis_);

		effectTransform_.rotate *= Quaternion::MakeFromAngleAxis(baseSpeed * 2.0f, effectAxis_);

		batterTransform_.translate.y = rnd_.NextFloatRange(0.01f, 0.015f);

		for (auto& material : insideModel_.GetMaterials()) {
			material->albedo = { 1.0f + rate, 1.0f - rate * 0.3f, 1.0f - rate };
		}
		/* for (auto& material : outsideModel_.GetMaterials()) {
			 material->albedo = { 1.0f, 1.0f, 1.0f - rate };
		 }*/
		for (auto& material : effectModel_.GetMaterials()) {
			material->albedo = { 1.0f + rate, 1.0f - rate * 0.3f, 1.0f - rate };
		}
	}
	else {
		static float time = 0.0f;

		time += 0.08f;

		if (time > 6.283185f) time -= 6.283185f;

		float rate = 0.05f;

		insideTransform_.rotate *= Quaternion::MakeFromAngleAxis(rate * 0.2f, insideAxis_);
		outsideTransform_.rotate *= Quaternion::MakeFromAngleAxis(rate * 0.1f, outsideAxis_);
		effectTransform_.rotate *= Quaternion::MakeFromAngleAxis(rate * 0.1f, effectAxis_);

		batterTransform_.translate.y = std::sinf(time) * 0.01f;

		for (auto& material : insideModel_.GetMaterials()) {
			material->albedo = { 1.0f, 1.0f, 1.0f };
		}
		for (auto& material : outsideModel_.GetMaterials()) {
			material->albedo = { 1.0f, 1.0f, 1.0f };
		}
		for (auto& material : effectModel_.GetMaterials()) {
			material->albedo = { 1.0f, 1.0f, 1.0f };
		}
	}

	batterTransform_.UpdateMatrix();
	insideTransform_.UpdateMatrix();
	outsideTransform_.UpdateMatrix();
	effectTransform_.UpdateMatrix();

	effectModel_.SetWorldMatrix(effectTransform_.worldMatrix);
	insideModel_.SetWorldMatrix(insideTransform_.worldMatrix);
	outsideModel_.SetWorldMatrix(outsideTransform_.worldMatrix);
	//batteryModel_.SetWorldMatrix(trolley_->GetBatteyTransform(0).worldMatrix);


	speedMeterModel_.SetWorldMatrix(speedMeterTransform_.worldMatrix);
	speedMeterNeedleModel_.SetWorldMatrix(speedMeterNeedleTransform_.worldMatrix);

	float chargeT = std::clamp(trolley_->GetCurrentCharge() / trolley_->GetMaxNormalCharge(), 0.0f, 1.0f);
	float nitroT = std::clamp(trolley_->GetNitroAccumulateTimer() / trolley_->GetNitroChargeTime(), 0.0f, 1.0f);

	chargeUI_.SetScale({ chargeUISize_.x, std::lerp(0.0f, chargeUISize_.y, chargeT) });
	nitroUI_.SetScale({ nitroUISize_.x,std::lerp(0.0f,  nitroUISize_.y, nitroT) });

	chargeUI_.SetUVRect({ {0.0f, 1.0f - chargeT}, {1.0f, chargeT} }, Sprite::UVMode::UV);
	nitroUI_.SetUVRect({ {0.0f, 1.0f - nitroT}, { 1.0f,nitroT} }, Sprite::UVMode::UV);

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

void TrolleyUI::SetIsActive(bool isActive) {
	//model
	speedMeterModel_.SetIsActive(isActive);
	speedMeterNeedleModel_.SetIsActive(isActive);
	effectModel_.SetIsActive(isActive);
	insideModel_.SetIsActive(isActive);
	outsideModel_.SetIsActive(isActive);
	batteryModel_.SetIsActive(isActive);

	//sprite
	chargeUI_.SetIsActive(isActive);
	baseUI_.SetIsActive(isActive);
	nitroUI_.SetIsActive(isActive);
	nitroBurstUI_.SetIsActive(isActive);
	baseUI_.SetIsActive(isActive);
}