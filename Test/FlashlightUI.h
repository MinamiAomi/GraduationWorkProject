#pragma once

#include "Math/Transform.h"
#include "Graphics/Model.h"
#include "Graphics/Sprite.h"


class Flashlight;
class FlashlightUI {
public:
	void Initialize();
	void Update();

	void SetFlashlight(const Flashlight* flashlight) { flashlight_ = flashlight; }
private:
	const Flashlight* flashlight_;

	ModelInstance model_;
	Transform transform_;

	Sprite batteryUI_;

};