#pragma once

#include <memory>
#include <string>

#include "Math/Transform.h"
#include "Collider.h"

#include "Graphics/Model.h"
#include "Graphics/Sprite.h"

#include "RailAnimationPlayer.h"


class TutorialObject {
public:

	void Initialize(const std::string& name, float frame);

	void Update();

	bool GetIsActive() { return sprite_.GetIsActive(); }

	void SetRailCameraPlayer(const RailSystem::RailAnimationPlayer* railAnimationPlayer) { railAnimationPlayer_ = railAnimationPlayer; }
private:

	void CheckInput();

	Sprite sprite_;
	Transform transform_;

	const RailSystem::RailAnimationPlayer* railAnimationPlayer_;

	float drawFrame_;

	bool isOnce_;

	float maxTime_;

	float currentTime_;

#ifdef _DEBUG
	std::string name_;
	void DrawImGui();
#endif // _DEBUG

};