#include "FlashlightUI.h"

#include "Framework/AssetManager.h"

#include "Flashlight.h"

#include "Math/Color.h"

FlashlightUI::FlashlightUI()
{
	auto assetManager = AssetManager::GetInstance();

	auto baseTexture = assetManager->textureMap.Get("FlashlightBase")->Get();
	auto frameTexture = assetManager->textureMap.Get("FlashlightBase")->Get();
	auto batteryTexture = assetManager->textureMap.Get("FlashlightBase")->Get();

	baseUI_.SetTexture(assetManager->textureMap.Get("FlashlightBase")->Get());
	frameUI_.SetTexture(assetManager->textureMap.Get("FlashlightFrame")->Get());
	batteryUI_.SetTexture(assetManager->textureMap.Get("FlashlightGauge")->Get());


	baseUI_.SetPosition({ 256.0f,80.0f });
	baseUI_.SetAnchor({ 0.5f,0.5f });
	baseUI_.SetScale(baseTexture->GetSize());
	baseUI_.SetUVRect({ {0.0f,0.0f },{1.0f,1.0f} }, Sprite::UVMode::UV);
	baseUI_.SetDrawOrder(0);


	frameUI_.SetPosition({ 180.0f,80.0f });
	frameUI_.SetAnchor({ 0.5f,0.5f });
	frameUI_.SetScale({ 280.0f,50.0f });
	frameUI_.SetUVRect({ {0.0f,0.0f },{1.0f,1.0f} }, Sprite::UVMode::UV);
	frameUI_.SetDrawOrder(2);

	batteryUI_.SetPosition({ 40.0f,80.0f });
	batteryUI_.SetAnchor({ 0.0f,0.5f });
	batteryUI_.SetScale({ 280.0f,50.0f });
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


	batteryUI_.SetScale({ std::lerp(0.0f,280.0f,t) ,50.0f });
	batteryUI_.SetUVRect({ {0.0f,0.0f },{t,1.0f} }, Sprite::UVMode::UV);

#ifdef _DEBUG
	baseUI_.DrawImGui("flashlightBase");
	frameUI_.DrawImGui("flashlightFrame");
	batteryUI_.DrawImGui("batteryUI");
#endif // _DEBUG

}