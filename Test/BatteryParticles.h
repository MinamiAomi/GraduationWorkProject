#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"

#include "ParticleDefine.h"
#include "BatsManager.h"

class Model;


class BatteryParticles
{
public:

	void Initialize(const Transform* transform, const BatsManager* batsManager);
	void Update();
	void DebugDraw();
	static void Debug();

	void SetColor(Vector3 color) {
		material_->albedo = color;
	};
	void SetRadius(float radius) { radius_ = radius; }

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
public:
	std::shared_ptr<Material> material_;
	std::shared_ptr<Material> toBatMaterial_;
	inline static float minSpeed_ = 0.002f;
	inline static float maxSpeed_ = 0.003f;
	inline static Vector3 minAngularVelocity_ = { -0.05f, -0.05f, -0.05f };
	inline static Vector3 maxAngularVelocity_ = { 0.05f,  0.05f,  0.05f };
	inline static float scaleDecay_ = 0.02f;
	inline static int emitInterval_ = 20;
	inline static float minScale_ = 0.1f;
	inline static float maxScale_ = 0.5f;
	inline static Vector3 color_ = { 1.0f,1.0f,1.0f };
	inline static float toBatSpeed_ = 0.01f;
	inline static float toBatScaleSpeed_ = 0.01f;
	//Sphere
	inline static float radius_;
private:
	void Emit();
	void ToBatEmit();
private:
	struct Particle {
		ModelInstance modelInstance_;
		Transform transform_;
		Vector3 angularVelocity_;
		Vector3 velocity_;
		float scaleSpeed_;
	};

	std::shared_ptr<Model> model_;

	const BatsManager* batsManager_ = nullptr;

	Transform transform_;
	std::vector<std::unique_ptr<Particle>> particles_;
	Random::RandomNumberGenerator rnd_;
	int emitTimer_ = 0;
};
