#include "RailcameraUI.h"

#include "Framework/AssetManager.h"
#include "Engine/File/JsonConverter.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

RailcameraUI::RailcameraUI()
{
	auto assetManager = AssetManager::GetInstance();

	auto frameBaseUI = assetManager->textureMap.Get("ProgressBaseUI")->Get();
	auto progressFrameUI = assetManager->textureMap.Get("ProgressFrame")->Get();
	auto progressGaugeUI = assetManager->textureMap.Get("ProgressGauge")->Get();

	baseUI_.SetTexture(frameBaseUI);
	progressFrameUI_.SetTexture(progressFrameUI);
	progressGaugeUI_.SetTexture(progressGaugeUI);

	baseUI_.SetPosition({ 640.0f,360.0f });
	baseUI_.SetScale(frameBaseUI->GetSize());
	baseUI_.SetAnchor({ 0.5f,0.5f });
	baseUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);

	progressFrameUI_.SetPosition({ 380.0f ,690.0f });
	progressFrameUI_.SetScale(progressFrameUI->GetSize());
	progressFrameUI_.SetAnchor({ 0.0f,0.5f });
	progressFrameUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);


	progressGaugeUI_.SetPosition({ 380.0f ,690.0f });
	progressGaugeUI_.SetScale(progressGaugeUI->GetSize());
	progressGaugeUI_.SetAnchor({ 0.0f,0.5f });
	progressGaugeUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
}

void RailcameraUI::Initialize()
{
}

void RailcameraUI::Update(float currentFrame)
{
	float t = std::clamp(currentFrame, 0.0f, 1.0f);

	progressGaugeUI_.SetScale({ std::lerp(0.0f, 520.0f, t), 50.0f });
	progressGaugeUI_.SetUVRect({ {0.0f, 0.0f}, {t, 1.0f} }, Sprite::UVMode::UV);

#ifdef _DEBUG
	progressGaugeUI_.DrawImGui("progressGaugeUI");
	progressFrameUI_.DrawImGui("progressFrameUI");
	baseUI_.DrawImGui("baseUI");
#endif // _DEBUG
}