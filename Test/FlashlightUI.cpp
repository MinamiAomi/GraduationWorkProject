#include "FlashlightUI.h"

#include "Framework/AssetManager.h"

#include "Flashlight.h"

#include "Math/Color.h"

#include "Scene/SceneManager.h"

#include "GameScene.h"

FlashlightUI::FlashlightUI()
{
	auto assetManager = AssetManager::GetInstance();

	auto flashlightFrame = assetManager->textureMap.Get("FlashlightFrame")->Get();
	auto flashlightGauge = assetManager->textureMap.Get("FlashlightGauge")->Get();

	frameUI_.SetTexture(flashlightFrame);
	batteryUI_.SetTexture(flashlightGauge);

	frameUI_.SetPosition({ 150.0f,60.0f });
	frameUI_.SetAnchor({ 0.5f,0.0f });
	frameUI_.SetScale({ flashlightFrame->GetSize() });
	frameUI_.SetUVRect({ {0.0f,0.0f },{1.0f,1.0f} }, Sprite::UVMode::UV);
	frameUI_.SetDrawOrder(2);
	frameUI_.SetIsActive(false);


	batteryUISize_ = flashlightGauge->GetSize();
	batteryUI_.SetPosition({ 150.0f,60.0f });
	batteryUI_.SetAnchor({ 0.5f,0.0f });
	batteryUI_.SetScale({ batteryUISize_ });
	batteryUI_.SetUVRect({ {0.0f,0.0f },{1.0f,1.0f} }, Sprite::UVMode::UV);
	batteryUI_.SetDrawOrder(1);

}

void FlashlightUI::Initialize()
{

}

void FlashlightUI::Update()
{

	float t = flashlight_->GetBattery() / flashlight_->GetMaxBattery();
	t = std::clamp(t, 0.0f, 1.0f);

	float maxHeight = batteryUISize_.y;

	batteryUI_.SetScale({ batteryUISize_.x, maxHeight * t });

	batteryUI_.SetUVRect({ {0.0f, 1.0f - t}, {1.0f, t} }, Sprite::UVMode::UV);
#ifdef _DEBUG
	frameUI_.DrawImGui("flashlightFrame");
	batteryUI_.DrawImGui("batteryUI");
#endif // _DEBUG

	if (dynamic_cast<GameScene*>(SceneManager::GetInstance()->GetCurrentScene())) {
		batteryUI_.SetIsActive(true);
		frameUI_.SetIsActive(true);
	}
	else {
		batteryUI_.SetIsActive(false);
		frameUI_.SetIsActive(false);
	}


}