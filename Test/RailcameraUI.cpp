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
	auto frameBaseOtherUI = assetManager->textureMap.Get("ProgressBaseOtherUI")->Get();
	auto trollyIcon = assetManager->textureMap.Get("TrollyIcon")->Get();
	auto deadLineIcon = assetManager->textureMap.Get("DeadLineIcon")->Get();
	auto sg = assetManager->textureMap.Get("SG")->Get();
	auto circle = assetManager->textureMap.Get("Circle")->Get();


	baseUI_.SetTexture(frameBaseUI);
	baseOtherUI_.SetTexture(frameBaseOtherUI);
	trollyIcon_.sprite.SetTexture(trollyIcon);
	deadLineIcon_.sprite.SetTexture(deadLineIcon);
	sg_.sprite.SetTexture(sg);

	baseUI_.SetPosition({ 640.0f,360.0f });
	baseUI_.SetScale(frameBaseUI->GetSize());
	baseUI_.SetAnchor({ 0.5f,0.5f });
	baseUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	baseUI_.SetDrawOrder(0);

	baseOtherUI_.SetPosition({ 640.0f,360.0f });
	baseOtherUI_.SetScale(frameBaseOtherUI->GetSize());
	baseOtherUI_.SetAnchor({ 0.5f,0.5f });
	baseOtherUI_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	baseOtherUI_.SetDrawOrder(0);
	baseOtherUI_.SetIsActive(false);

	trollyIcon_.position = { 640.0f, 690.0f };
	trollyIcon_.size = trollyIcon->GetSize();
	trollyIcon_.sprite.SetPosition(trollyIcon_.position);
	trollyIcon_.sprite.SetScale(trollyIcon_.size);
	trollyIcon_.sprite.SetAnchor({ 0.5f,0.5f });
	trollyIcon_.sprite.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	trollyIcon_.sprite.SetDrawOrder(1);
	trollyIcon_.sprite.SetIsActive(false);


	deadLineIcon_.size = deadLineIcon->GetSize();
	deadLineIcon_.position = { 640.0f ,690.0f };
	deadLineIcon_.sprite.SetPosition(deadLineIcon_.position);
	deadLineIcon_.sprite.SetScale(deadLineIcon_.size);
	deadLineIcon_.sprite.SetAnchor({ 0.5f,0.5f });
	deadLineIcon_.sprite.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	deadLineIcon_.sprite.SetDrawOrder(2);
	deadLineIcon_.sprite.SetIsActive(false);

	sg_.size = sg->GetSize();
	sg_.position = { 640.0f,686.0f };
	sg_.sprite.SetPosition(sg_.position);
	sg_.sprite.SetScale(sg_.size);
	sg_.sprite.SetAnchor({ 0.5f,0.5f });
	sg_.sprite.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	sg_.sprite.SetDrawOrder(0);
	sg_.sprite.SetIsActive(false);

	for (auto& spriteData : circleSprite_) {
		spriteData.size = circle->GetSize();
		spriteData.position = { 1280.0f * 0.5f,720.0f * 0.5f };
		spriteData.sprite.SetTexture(circle);
		spriteData.sprite.SetAnchor({ 0.5f,0.5f });
		spriteData.sprite.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
		spriteData.sprite.SetIsActive(false);
	}
}

void RailcameraUI::Initialize()
{
	flashTimer_ = 0.0f;
	animationOffset_ = { 0.0f,-250.0f };
	animationSize_ = 1.5f;

	circleStratPosition_ = { 390.0f,685.0f };
	circleEndPosition_ = { 890.0f,685.0f };
	isAnimation_ = false;
	isCircleSprite_ = false;
}

void RailcameraUI::Update(float currentTrollyFrame, float currentDeadlineFrame, bool startWarning)
{
	float trollyT = std::clamp(currentTrollyFrame, 0.0f, 1.0f);
	float deadlineT = std::clamp(currentDeadlineFrame, 0.0f, 1.0f);

	trollyIcon_.sprite.SetUVRect({ {std::lerp(0.0f,-0.95f,trollyT),0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	deadLineIcon_.sprite.SetUVRect({ {std::lerp(0.0f,-0.95f,deadlineT),0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);




	//赤く点滅
	if (startWarning) {
		float distance = std::abs(trollyT - deadlineT);

		float threshold = 0.03f;
		Vector4 uiColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		Vector4 uiOtherColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		if (distance < threshold) {
			flashTimer_ += 0.1f;

			float sinValue = (std::sin(flashTimer_) + 1.0f) * 0.5f;

			//float alertIntensity = 1.0f - (distance / threshold);

			float g_b_ratio = 1.0f - (sinValue);
			uiColor.y = g_b_ratio;
			uiColor.z = g_b_ratio;
			uiOtherColor = uiColor;
			uiOtherColor.w = sinValue;
			baseOtherUI_.SetIsActive(true);
		}
		else {
			flashTimer_ = 0.0f;
			baseOtherUI_.SetIsActive(false);
		}
		baseUI_.SetColor(Color(uiColor));

		baseOtherUI_.SetColor(Color(uiOtherColor));
	}

	if (isAnimation_) {
		trollyIcon_.sprite.SetIsActive(true);
		deadLineIcon_.sprite.SetIsActive(true);
		sg_.sprite.SetIsActive(true);
		float t = std::clamp(animationTimer_ / animationEndTime_, 0.0f, 1.0f);


		trollyIcon_.sprite.SetPosition(Vector2::Lerp(t, (trollyIcon_.position + animationOffset_), trollyIcon_.position));
		deadLineIcon_.sprite.SetPosition(Vector2::Lerp(t, (deadLineIcon_.position + animationOffset_), deadLineIcon_.position));
		sg_.sprite.SetPosition(Vector2::Lerp(t, (sg_.position + animationOffset_), sg_.position));

		trollyIcon_.sprite.SetScale(Vector2::Lerp(t, (trollyIcon_.size * animationSize_), trollyIcon_.size));
		deadLineIcon_.sprite.SetScale(Vector2::Lerp(t, (deadLineIcon_.size * animationSize_), deadLineIcon_.size));
		sg_.sprite.SetScale(Vector2::Lerp(t, (sg_.size * animationSize_), sg_.size));

		animationTimer_ += 1.0f;

		if (animationTimer_ == animationEndTime_) {
			isCircleSprite_ = true;
		}
	}

	if (isCircleSprite_) {
		float cycleTime = 180.0f;
		float dangerLevel = 0.0f;
		float speed = 1.0f;

		if (startWarning) {
			float distance = std::abs(trollyT - deadlineT);
			float threshold = 0.03f;
			if (distance < threshold) {
				dangerLevel = 1.0f - std::clamp(distance / threshold, 0.0f, 1.0f);
				speed = 1.0f + (dangerLevel * 2.0f);
			}
		}

		animationTime_ = std::fmod(animationTime_ + speed, cycleTime);

		int activeCircles = 3 + static_cast<int>(dangerLevel * (CircleNum - 3));
		float currentInterval = cycleTime / activeCircles;

		for (int i = 0; i < CircleNum; ++i) {
			if (i >= activeCircles) {
				circleSprite_[i].sprite.SetIsActive(false);
				continue;
			}

			float startTimeForThisCircle = i * currentInterval;
			if (animationTime_ < startTimeForThisCircle) {
				circleSprite_[i].sprite.SetIsActive(false);
				continue;
			}

			circleSprite_[i].sprite.SetIsActive(true);

			float t = (animationTime_ - startTimeForThisCircle) / cycleTime;

			float maxScale = 2.0f + (dangerLevel * 4.0f);
			Vector2 currentSize = circleSprite_[i].size * (1.0f + t * maxScale);
			circleSprite_[i].sprite.SetScale(currentSize);

			float rb = 1.0f - (dangerLevel * 0.8f);
			float alpha = (1.0f - t) * (0.2f + dangerLevel * 0.6f);
			circleSprite_[i].sprite.SetColor({ 1.0f, rb, rb, alpha });

			circleCurrentPosition_ = Vector2::Lerp(deadlineT, circleStratPosition_, circleEndPosition_);
			circleSprite_[i].sprite.SetPosition(circleCurrentPosition_);
		}
	}

#ifdef _DEBUG
	trollyIcon_.sprite.DrawImGui("trollyIcon");
	deadLineIcon_.sprite.DrawImGui("deadLineIcon");
	sg_.sprite.DrawImGui("sg");
	baseOtherUI_.DrawImGui("baseOtherUI");
	ImGui::DragFloat2("animationOffset", &animationOffset_.x);
	ImGui::DragFloat("animationSize", &animationSize_, 0.1f);
	ImGui::DragFloat2("circlePosition_", &circleCurrentPosition_.x);
#endif // _DEBUG
}