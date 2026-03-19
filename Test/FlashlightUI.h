#pragma once

#include "Math/Transform.h"
#include "Graphics/Model.h"
#include "Graphics/Sprite.h"

#include "Math/Random.h"

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
	Vector2 batteryUIPosition_;

	Random::RandomNumberGenerator rnd_;
};