#include "FlashlightUI.h"

#include "Framework/AssetManager.h"

#include "Flashlight.h"

#include "Math/Color.h"

FlashlightUI::FlashlightUI()
{
	batteryUI_.SetTexture(AssetManager::GetInstance()->textureMap.Get("white2x2")->Get());
	batteryUI_.SetScale({ 45.0f,650.0f });
	batteryUI_.SetAnchor({ 0.5f, 0.0f });
	batteryUI_.SetPosition({ 50.0f, 25.0f });
	batteryUI_.SetColor(Color::Convert(Color::HSVA(0.34f, 0.6f, 0.7f)));
}

void FlashlightUI::Initialize()
{
}

void FlashlightUI::Update()
{
	float t = flashlight_->GetBattery() / flashlight_->GetMaxBattery();
	t = std::clamp(t, 0.0f, 1.0f);
	batteryUI_.SetScale({ 45.0f,std::lerp(0.0f,650.0f,t) });
	batteryUI_.SetColor(Color::Convert(Color::HSVA(std::lerp(0.0f, 0.3f, t), 0.6f, 0.7f)));

#ifdef _DEBUG
	batteryUI_.DrawImGui("batteryUI");
#endif // _DEBUG

}