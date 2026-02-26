#pragma once

#include "Math/MathUtils.h"

#include "Graphics/Sprite.h"

class TextUI {
public:
	TextUI();
	void Initialize(const Vector2& pos);
	void Update();

private:
	float time_ = 0.0f;
	Vector2 size_;
	Sprite sprite_;
};