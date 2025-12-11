#pragma once

#include "Graphics/Model.h"

#include "Math/Transform.h"

class Deadline;
class DeadlineUI {
public:
	DeadlineUI();
	void Initialize();
	void Update();

	void SetDeadline(const Deadline* deadline) { deadline_ = deadline; }
private:
	const Deadline* deadline_;
	ModelInstance  monster_;
	Transform  monsterTransform_;
	float monsterRotateTimer_;
};