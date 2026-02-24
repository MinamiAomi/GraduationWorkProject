#include "TutorialObject.h"

#include "Framework/AssetManager.h"

#include "GameSystem.h"
#include "LightDeviceInput.h"
#include "Input/Input.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

void TutorialObject::Initialize(const std::string& name, float frame)
{


#ifdef _DEBUG
	name_ = name;
#endif // _DEBUG

	auto assetManager = AssetManager::GetInstance();

	auto texture = assetManager->textureMap.Get(name)->Get();

	sprite_.SetTexture(texture);
	sprite_.SetColor(Color::white);
	sprite_.SetPosition({ 1280.0f * 0.5f, 720.0f * 0.5f });
	sprite_.SetScale(texture->GetSize());
	sprite_.SetAnchor({ 0.5f, 0.5f });
	sprite_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	sprite_.SetIsActive(false);
	sprite_.SetDrawOrder(5);

	isOnce_ = false;

	drawFrame_ = frame;

	maxTime_ = 120.0f;
	currentTime_ = maxTime_;
}

void TutorialObject::Update()
{
	//一度出現してスプライトが出ていなけらば用済み
	if (isOnce_ && !sprite_.GetIsActive()) {
		return;
	}

	if (!isOnce_ && railAnimationPlayer_->GetCurrentFrame() >= drawFrame_) {

		isOnce_ = true;

		sprite_.SetIsActive(true);

		transform_.SetParent(&railAnimationPlayer_->GetTransform());
		transform_.UpdateMatrix();

	}
	if (sprite_.GetIsActive()) {
		CheckInput();
	}


#ifdef _DEBUG
	DrawImGui();
#endif // _DEBUG

}

void TutorialObject::CheckInput()
{
	GameSystem* gameSystem = GameSystem::GetInstance();
	Input* input = Input::GetInstance();

	auto playDevice = gameSystem->GetPlayDevice();
	switch (playDevice)
	{
	case GameSystem::PlayDevice::KeyboardMouse: {
		if (input->IsMouseTrigger(0)) {
			sprite_.SetIsActive(false);
		}

		break;
	}
	case GameSystem::PlayDevice::LightDevice: {
		LightDeviceInput* lightDeviceInput = LightDeviceInput::GetInstance();
		if (lightDeviceInput->GetConnectionState() == LightDeviceInput::ConnectionState::Connected) {
			if (lightDeviceInput->IsButtonPressed()) {
				sprite_.SetIsActive(false);
			}
		}
		break;
	}
	}
}

#ifdef _DEBUG
void TutorialObject::DrawImGui()
{
	transform_.UpdateMatrix();

	sprite_.DrawImGui(name_ + "sprite");
}
#endif // _DEBUG
