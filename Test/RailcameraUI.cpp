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
	auto trollyIcon = assetManager->textureMap.Get("TrollyIcon")->Get();
	auto deadLineIcon = assetManager->textureMap.Get("DeadLineIcon")->Get();

	baseUI_.SetTexture(frameBaseUI);
	trollyIcon_.SetTexture(trollyIcon);
	deadLineIcon_.SetTexture(deadLineIcon);

	baseUI_.SetPosition({ 640.0f,360.0f });
	baseUI_.SetScale(frameBaseUI->GetSize());
	baseUI_.SetAnchor({ 0.5f,0.5f });
	baseUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);

	trollyIcon_.SetPosition({ 640.0f ,690.0f });
	trollyIcon_.SetScale(trollyIcon->GetSize());
	trollyIcon_.SetAnchor({ 0.5f,0.5f });
	trollyIcon_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);


	deadLineIcon_.SetPosition({ 640.0f ,690.0f });
	deadLineIcon_.SetScale(deadLineIcon->GetSize());
	deadLineIcon_.SetAnchor({ 0.5f,0.5f });
	deadLineIcon_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
}

void RailcameraUI::Initialize()
{
}

void RailcameraUI::Update(float currentTrollyFrame,float currentDeadlineFrame)
{
	float trollyT = std::clamp(currentTrollyFrame, 0.0f, 1.0f);
	float deadlineT = std::clamp(currentDeadlineFrame, 0.0f, 1.0f);
	trollyIcon_.SetUVRect({ {std::lerp(0.0f,-0.95f,trollyT),0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	deadLineIcon_.SetUVRect({ {std::lerp(0.0f,-0.95f,deadlineT),0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);

#ifdef _DEBUG
	trollyIcon_.DrawImGui("trollyIcon");
	deadLineIcon_.DrawImGui("deadLineIcon");
	baseUI_.DrawImGui("baseUI");
#endif // _DEBUG
}