#pragma once

#include "Graphics/Sprite.h"

class RailcameraUI {
public:
	RailcameraUI();

	void Initialize();

	void Update(float currentFrame);


private:
	Sprite progressFrameUI_;
	Sprite progressGaugeUI_;

	Sprite baseUI_;
};