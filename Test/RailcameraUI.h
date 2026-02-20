#pragma once

#include "Graphics/Sprite.h"

class RailcameraUI {
public:
	RailcameraUI();

	void Initialize();

	void Update(float currentTrollyFrame, float currentDeadlineFrame, bool startWarning);


private:
	Sprite trollyIcon_;
	Sprite deadLineIcon_;

	Sprite baseUI_;
	float flashTimer_;
};