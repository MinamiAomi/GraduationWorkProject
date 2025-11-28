#pragma once

#include "Math/Transform.h"
#include "Math/MathUtils.h"
#include "Graphics/Model.h"

class Trolley;
class TrolleyUI {
public:
	void Initialize(const Transform& transform);
	void Update();

	void SetTrolley(const Trolley* trolley) { trolley_ = trolley; }
private:
	const Trolley* trolley_;

	ModelInstance speedMeterModel_;
	ModelInstance needleModel_;

	Transform speedMeterTransform_;
	Transform needleTransform_;

	Vector3 speedMeterOffset_;
	Vector3 needleOffset_;
};