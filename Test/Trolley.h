#pragma once
#include "Engine/Graphics/Model.h"

#include "Math/MathUtils.h"
#include "Math/Transform.h"

class Trolley {
public:
	Trolley();

	void Initialize();
	void Update();

	void SetTransform(const Transform& transform);
private:
	ModelInstance model_;

	Transform transform_;

	Vector3 offset_;
};