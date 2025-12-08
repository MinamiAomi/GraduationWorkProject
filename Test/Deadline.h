#pragma once

#include "DeadlineUI.h"

namespace RailSystem {
	class RailAnimationPlayer;
}

class Deadline {
public:
	Deadline();

	void Initialize();
	void Update(float deltaTime);

	void SetAnimationPlayer(const RailSystem::RailAnimationPlayer* railAnimationPlayer) { railAnimationPlayer_ = railAnimationPlayer; }
	const RailSystem::RailAnimationPlayer* GetAnimationPlayer() const  {return railAnimationPlayer_; }
	float GetCurrenFrame() const { return currentFrame_; }

	bool IsGameOver() { return isGameOver_; }
private:
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG

	DeadlineUI deadlineUI_;
	const RailSystem::RailAnimationPlayer* railAnimationPlayer_;
	float currentFrame_;
	float startFrame_;
	bool isGameOver_;


	float minSpeed_ = 1.0f;       
	float maxSpeed_ = 5.0f;    

	float approachRate_ = 0.5f;

	float currentActualSpeed_ = 0.0f;
};