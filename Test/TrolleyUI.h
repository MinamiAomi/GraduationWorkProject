#pragma once

#include "Math/Transform.h"
#include "Math/MathUtils.h"
#include "Math/Random.h"
#include "Graphics/Model.h"
#include "Graphics/Sprite.h"


class Trolley;
class TrolleyUI {
public:
	TrolleyUI();
	void Initialize(const Transform& transform);
	void Update();

	void SetTrolley(const Trolley* trolley) { trolley_ = trolley; }
	void SetIsActive(bool isActive);
    const Transform& GetBatterTransform() const { return batterTransform_; }

private:
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG

	void UpdateSprite(Sprite& sprite, float t, float size);

	const Trolley* trolley_;

	ModelInstance speedMeterModel_;
	ModelInstance speedMeterNeedleModel_;

	ModelInstance effectModel_;
	ModelInstance insideModel_;
	ModelInstance outsideModel_;
	ModelInstance batteryModel_;

	Transform speedMeterTransform_;
	Transform speedMeterNeedleTransform_;

	Transform batterTransform_;

	Transform insideTransform_;
	Transform outsideTransform_;
	Transform effectTransform_;

	Vector3 insideAxis_ = Vector3(1, 0.5f, 0).Normalized();
	Vector3 outsideAxis_ = Vector3(0, 1, 0.5f).Normalized();
	Vector3 effectAxis_ = Vector3(0.5f, 0, 1).Normalized();

	Vector3 speedMeterOffset_;
	Vector3 speedMeterNeedleOffset_;

	Sprite baseUI_;

	Sprite chargeUI_;
	Sprite nitroUI_;
	Sprite nitroBurstUI_;


	Vector2 chargeUISize_;
	Vector2 nitroUISize_;
	Vector2 nitroBurstUISize_;

	Random::RandomNumberGenerator rnd_;
};