#pragma once

#include "Graphics/Sprite.h"

class RailcameraUI {
public:
	RailcameraUI();

	void Initialize();

	void Update(float currentTrollyFrame, float currentDeadlineFrame, bool startWarning);

	void SetStartAnimation(bool flag) { isAnimation_ = flag; }
private:
	struct SpriteData {
		Sprite sprite;
		Vector2 size;
		Vector2 position;
	};

	SpriteData trollyIcon_;
	SpriteData deadLineIcon_;
	SpriteData sg_;

	Sprite baseUI_;
	Sprite baseOtherUI_;
	float flashTimer_;

	bool isAnimation_;

	Vector2 animationOffset_;
	float animationSize_;

	float animationTimer_;
	float animationEndTime_ = 120.0f;
};