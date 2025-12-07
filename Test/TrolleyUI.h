#pragma once

#include "Math/Transform.h"
#include "Math/MathUtils.h"
#include "Graphics/Model.h"
#include "Graphics/Sprite.h"
class Trolley;
class TrolleyUI {
public:
	void Initialize(const Transform& transform);
	void Update();

	void SetTrolley(const Trolley* trolley) { trolley_ = trolley; }
private:
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG

	const Trolley* trolley_;

	ModelInstance speedMeterModel_;
	ModelInstance speedMeterNeedleModel_;

	Transform speedMeterTransform_;
	Transform speedMeterNeedleTransform_;

	Vector3 speedMeterOffset_;
	Vector3 speedMeterNeedleOffset_;

	Sprite chargeUI_;
	Sprite overChargeUI_;
	Sprite nitroUI_;
	Sprite nitroBurstUI_;

};