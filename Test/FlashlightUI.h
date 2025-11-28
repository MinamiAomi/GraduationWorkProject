#pragma once

#include "Math/Transform.h"
#include "Graphics/Model.h"

class FlashlightUI {
public:
	void Initialize();
	void Update();
private :
	ModelInstance model_;
	Transform transform_;
};