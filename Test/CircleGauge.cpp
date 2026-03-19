#include "CircleGauge.h"

#include "Framework/AssetManager.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

CircleGauge::CircleGauge()
{
	auto assetManager = AssetManager::GetInstance();

	/*const std::string assetNames[GaugePart::Count] = {
		"CircleFrame", "RightGauge", "LeftGauge", "HideGauge"
	};*/

	//for (int i = 0; i < GaugePart::Count; ++i) {
	//	auto texture = assetManager->textureMap.Get(assetNames[i])->Get();

	//	gaugeParts_[i].spriteScale = texture->GetSize();

	//	Vector2 spriteOffset = Vector2::zero;


	//	switch (GaugePart(i))
	//	{
	//	case CircleGauge::Right:
	//		gaugeParts_[i].sprite.SetAnchor({ 0.0f, 0.5f });
	//		break;
	//	case CircleGauge::Hide:
	//	case CircleGauge::Left:
	//		gaugeParts_[i].sprite.SetAnchor({ 1.0f, 0.5f });
	//		break;
	//	}

	//	gaugeParts_[i].offset = spriteOffset;

	//	gaugeParts_[i].sprite.SetTexture(texture);
	//	gaugeParts_[i].sprite.SetScale(gaugeParts_[i].spriteScale);
	//	gaugeParts_[i].sprite.SetDrawOrder(uint8_t(i));
	//	gaugeParts_[i].sprite.SetUVRect({ {0.0f, 0.0f}, {1.0f, 1.0f} }, Sprite::UVMode::UV);

	//}

	auto texture = assetManager->textureMap.Get("CircleGauge")->Get();
	gauge_.spriteScale = texture->GetSize();

	gauge_.sprite.SetTexture(texture);
	gauge_.sprite.SetScale(gauge_.spriteScale);
	gauge_.sprite.SetAnchor({ 0.5f,0.5f });
	gauge_.sprite.SetDrawOrder(uint8_t(0));
	gauge_.sprite.SetUVRect({ {0.0f, 0.0f}, {1.0f, 1.0f} }, Sprite::UVMode::UV);
	gauge_.sprite.SetIsCircleGauge(true);
	//gauge_.sprite.SetColor(Color({ (200.0f / 255.0f),(210.0f/255.0f),(30.0f/255.0f)}));

}

void CircleGauge::Initialize(float size, const Vector2& screenPos)
{
	//for (auto& gaugePart : gaugeParts_) {
	//	gaugePart.sprite.SetScale(gaugePart.spriteScale * size);
	//}
	gaugeBasePosition_ = screenPos;
	gauge_.sprite.SetPosition(gaugeBasePosition_ + gauge_.offset);
	gauge_.sprite.SetScale(gauge_.spriteScale * size_);

#ifdef _DEBUG
	size_ = size;
#endif // _DEBUG

}

void CircleGauge::Update(float progress)
{
#ifdef _DEBUG
	DebugDraw();
#endif // _DEBUG
	//float p = std::clamp(progress, 0.0f, 1.0f);

	/*for (int i = 0; i < GaugePart::Count; ++i) {
		auto& part = gaugeParts_[i];

		if (p <= 0.0f) {
			part.sprite.SetIsActive(false);
			continue;
		}
		else {
			part.sprite.SetIsActive(true);
		}

		switch (static_cast<GaugePart>(i)) {
		case GaugePart::Right: {
			float t = std::clamp(p / 0.5f, 0.0f, 1.0f);
			part.sprite.SetRotate(std::lerp(180.0f * Math::ToRadian, 0.0f * Math::ToRadian, t));
			break;
		}
		case GaugePart::Left: {
			float t = std::clamp((p - 0.5f) / 0.5f, 0.0f, 1.0f);
			part.sprite.SetRotate(std::lerp(180.0f * Math::ToRadian, 0.0f * Math::ToRadian, t));
			if (p > 0.5f) {
				part.sprite.SetIsActive(true);
			}
			else {
				part.sprite.SetIsActive(false);
			}
			break;
		}
		case GaugePart::Hide:
			if (p <= 0.5f) {
				part.sprite.SetIsActive(true);
			}
			else {
				part.sprite.SetIsActive(false);
			}
			break;
		}

		part.sprite.SetPosition(gaugeBasePosition_ + part.offset);
	}*/
	
	
	gauge_.sprite.SetCircleGaugeProgress(progress);
	gauge_.sprite.SetPosition(gaugeBasePosition_ + gauge_.offset);
	gauge_.sprite.SetScale(gauge_.spriteScale * size_);
}

#ifdef _DEBUG
void CircleGauge::DebugDraw()
{

	gauge_.sprite.DrawImGui("gauge");
	ImGui::Begin("CircleGauge");
	ImGui::DragFloat("size", &size_, 0.1f);
	ImGui::DragFloat2("basePos", &gaugeBasePosition_.x, 0.1f);
	/*for (int i = 0; i < GaugePart::Count; ++i) {
		gaugeParts_[i].sprite.DrawImGui("GaugePart" + std::to_string(i));
		ImGui::DragFloat2(("GaugePart" + std::to_string(i) + "offset").c_str(), &gaugeParts_[i].offset.x);
		gaugeParts_[i].sprite.SetPosition(gaugeBasePosition_ + gaugeParts_[i].offset);
		gaugeParts_[i].sprite.SetScale(gaugeParts_[i].spriteScale * size_);
	}*/

	gauge_.sprite.SetPosition(gaugeBasePosition_ + gauge_.offset);
	gauge_.sprite.SetScale(gauge_.spriteScale * size_);
	ImGui::End();

}
#endif // _DEBUG
