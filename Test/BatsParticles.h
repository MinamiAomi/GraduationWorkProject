#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"
#include "ParticleDefine.h"

class BatsParticles
{
public:

	bool static isDebug;

	static void Debug();

	void Initialize(float radius);
	void Update();
	void DebugDraw();

	void SetColor(Vector3 color) {
		material_->albedo = color;
	};
	bool HasParticles() const {
		return !particles_.empty();
	}
	void SetSize(const Vector3& size) { size_ = size; }
	void SetRadius(float radius) { radius_ = radius; }

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
	void SetIsEmit(bool isEmit) { isEmit_ = isEmit; }
	void SetParent(Transform* parent) { transform_.SetParent(parent); }
public:
	std::shared_ptr<Material> material_;
	std::shared_ptr<Material> absorptionMaterial_;
	inline static float minSpeed_ = 0.002f;
	inline static float maxSpeed_ = 0.003f;
	inline static Vector3 minAngularVelocity_ = { -0.05f, -0.05f, -0.05f };
	inline static Vector3 maxAngularVelocity_ = { 0.05f,  0.05f,  0.05f };
	inline static Vector3 minDirection_ = { -0.5f, 1.0f, -0.5f };
	inline static Vector3 maxDirection_ = { 0.5f,  1.0f,  0.5f };
	inline static float startScaleDecay_ = 0.02f;
	inline static float goalScaleDecay_ = 0.02f;
	inline static int emitInterval_ = 20;
	inline static float minScale_ = 0.1f;
	inline static float maxScale_ = 0.5f;
	inline static Vector3 color_ = Vector3{ 0.46f,0.94f,1.0f };
	inline static Vector3 goalColor_ = Vector3{ 0.75f,0.2f,0.75f };

	float scaleDecay_;
private:
	void Emit();
	void AbsorptionEmit();
private:
	struct Particle {
		ModelInstance modelInstance_;
		Transform transform_;
		Vector3 angularVelocity_;
		Vector3 velocity_;
		float scaleSpeed_;
		bool isSuction_ = false;
	};

	bool isEmit_ = false;
	Transform transform_;
	std::shared_ptr<Model> model_;
	std::vector<std::unique_ptr<Particle>> particles_;
	Random::RandomNumberGenerator rnd_;
	EmitShape emitShapeType_; 
	int emitTimer_ = 0;
public:
	bool isDead_ = false;


	//OOBB
	Vector3 size_;
	//Sphere
	float radius_;
};
