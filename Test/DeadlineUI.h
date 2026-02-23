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

	const bool GetIsAnimationEnd() const { return isAnimationEnd_; }
private:
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG

	const Deadline* deadline_;
	ModelInstance  monster_;
	Transform  monsterTransform_;
	float monsterRotateTimer_;

	Sprite warning_;
	float startLevel1Warning_;
	float startLevel2Warning_;
	int count_ = 0;
	int maxCount_ = 150;
	bool isOnce_;
	bool isAnimationEnd_;
};