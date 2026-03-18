#pragma once

#include <array>

#include "Math/MathUtils.h"
#include "Graphics/Sprite.h"

class CircleGauge {
public:
	CircleGauge();
	void Initialize(float size,const Vector2& screenPos);

	void Update(float progress);
private:
#ifdef _DEBUG
	void DebugDraw();
	float size_;
#endif // _DEBUG


	enum GaugePart {
		Frame,
		Right,
		Left,
		Hide,
		Count,
	};

	struct SpriteElement {
		Sprite sprite;
		Vector2 spriteScale;
		Vector2 offset;
	};

	std::array<SpriteElement, GaugePart::Count> gaugeParts_;

	Vector2 gaugeBasePosition_;
};