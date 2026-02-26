#include "TextUI.h"

#include "Framework/AssetManager.h"

TextUI::TextUI()
{
	auto texture = AssetManager::GetInstance()->textureMap.Get("Bottom")->Get();
	sprite_.SetTexture(texture);
	size_ = texture->GetSize();
}

void TextUI::Initialize(const Vector2& pos)
{
	sprite_.SetPosition(pos);
	sprite_.SetAnchor({ 0.5f,0.5f });
	sprite_.SetScale(size_);
	sprite_.SetAnchor({ 0.5f,0.0f });
	sprite_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	sprite_.SetDrawOrder(3);
}

void TextUI::Update()
{
	time_ = std::fmod(time_ + 0.001f, 1.0f);
	sprite_.SetUVRect({ {time_,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
#ifdef _DEBUG
	sprite_.DrawImGui("TextUI");
#endif // _DEBUG

}
