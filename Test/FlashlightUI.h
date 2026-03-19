#pragma once

#include "Math/Transform.h"
#include "Graphics/Model.h"
#include "Graphics/Sprite.h"
#include "SpriteParticles.h"
#include "SpriteParticles.h"


class Flashlight;
class FlashlightUI {
public:
	FlashlightUI();
	void Initialize();
	void Update();

	void SetFlashlight(const Flashlight* flashlight) { flashlight_ = flashlight; }
private:
	const Flashlight* flashlight_;

	ModelInstance model_;
	Transform transform_;

	Sprite frameUI_;
	Sprite batteryUI_;
	Vector2 batteryUISize_;
	SpriteParticles sprite_;

};