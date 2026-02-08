#pragma once

#include "Graphics/Sprite.h"

class RailcameraUI {
public:
	RailcameraUI();

	void Initialize();

	void Update(float currentTrollyFrame, float currentDeadlineFrame);


private:
	Sprite trollyIcon_;
	Sprite deadLineIcon_;

	Sprite baseUI_;
};