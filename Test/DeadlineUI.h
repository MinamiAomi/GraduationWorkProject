#pragma once

#include "Graphics/Model.h"
#include "Graphics/Sprite.h"

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

	Sprite warning_;
	int count_ = 0;

};