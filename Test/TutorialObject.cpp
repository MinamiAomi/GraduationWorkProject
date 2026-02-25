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

	texture = assetManager->textureMap.Get("Pusing")->Get();
	flashLightSpriteSize_ = texture->GetSize();
	currentRect_ = 0.0f;
	flashLightSprite_.SetTexture(texture);
	flashLightSprite_.SetPosition({ 1280.0f * 0.5f, 110.0f });
	flashLightSprite_.SetScale({ flashLightSpriteSize_.x * 0.5f, flashLightSpriteSize_.y });
	flashLightSprite_.SetAnchor({ 0.5f, 0.5f });
	flashLightSprite_.SetUVRect({ {currentRect_,0.0f} ,{0.5f,1.0f} }, Sprite::UVMode::UV);
	flashLightSprite_.SetIsActive(false);
	flashLightSprite_.SetDrawOrder(6);

	isOnce_ = false;

	isActive_ = false;

	drawFrame_ = frame;

	maxTime_ = 120.0f;
	currentTime_ = maxTime_;
}

void TutorialObject::Update()
{
	//一度出現してスプライトが出ていなけらば用済み
	if (isOnce_ && !sprite_.GetIsActive()) {
		if (!isActive_) {
			return;
		}
		GameSystem* gameSystem = GameSystem::GetInstance();
		Input* input = Input::GetInstance();
		auto playDevice = gameSystem->GetPlayDevice();
		switch (playDevice)
		{
		case GameSystem::PlayDevice::KeyboardMouse: {
			if (input->IsMouseRelease(0)) {
				isActive_ = false;
			}

			break;
		}
		case GameSystem::PlayDevice::LightDevice: {
			LightDeviceInput* lightDeviceInput = LightDeviceInput::GetInstance();
			if (lightDeviceInput->GetConnectionState() == LightDeviceInput::ConnectionState::Connected) {
				if (lightDeviceInput->IsButtonRelease()) {
					isActive_ = false;
				}
			}
			break;
		}
		}
		return;
	}
	//出現
	if (!isOnce_ && railAnimationPlayer_->GetCurrentFrame() >= drawFrame_) {

		isOnce_ = true;
		isActive_ = true;
		sprite_.SetIsActive(true);
		flashLightSprite_.SetIsActive(true);

		transform_.SetParent(&railAnimationPlayer_->GetTransform());
		transform_.UpdateMatrix();

	}
	if (sprite_.GetIsActive()) {
		animationTime_ = std::fmod(animationTime_ + 1.0f, (60.0f));
		if (animationTime_ == 0.0f) {
			currentRect_ = (currentRect_ == 0.0f) ? 0.5f : 0.0f;

		}


		flashLightSprite_.SetUVRect({ {currentRect_,0.0f} ,{0.5f,1.0f} }, Sprite::UVMode::UV);
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
			flashLightSprite_.SetIsActive(false);
		}

		break;
	}
	case GameSystem::PlayDevice::LightDevice: {
		LightDeviceInput* lightDeviceInput = LightDeviceInput::GetInstance();
		if (lightDeviceInput->GetConnectionState() == LightDeviceInput::ConnectionState::Connected) {
			if (lightDeviceInput->IsButtonTrigger()) {
				sprite_.SetIsActive(false);
				flashLightSprite_.SetIsActive(false);
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
	flashLightSprite_.DrawImGui("flashLightSprite_");
}
#endif // _DEBUG
