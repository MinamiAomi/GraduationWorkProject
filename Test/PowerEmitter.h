#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"
#include "LightObject.h"
#include "ParticleDefine.h"


class PowerEmitter
{
public:

	bool static isDebug;

	void Initialize(EmitShape shape, const LightObject* parentLight);
	void Update();
	void DebugDraw();
	void Debug();

	void SetColor(Vector3 color) {
		material_->albedo = color;
	};
	void SetSize(const Vector3& size) { size_ = size; }
	void SetRadius(float radius) { radius_ = radius; }

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
public:
	std::shared_ptr<Material> material_;
	float minSpeed_ = 0.002f;
	float maxSpeed_ = 0.003f;
	Vector3 minAngularVelocity_ = { -0.05f, -0.05f, -0.05f };
	Vector3 maxAngularVelocity_ = { 0.05f,  0.05f,  0.05f };
	float scaleDecay_ = 0.02f;
	int emitInterval_ = 20;
	float minScale_ = 0.1f;
	float maxScale_ = 0.5f;
private:
	void Emit();
private:
	struct Particle {
		ModelInstance modelInstance_;
		Transform transform_;
		Vector3 angularVelocity_;
		Vector3 velocity_;
		float scaleSpeed_;
		bool isSuction_ = false;
	};


	Transform transform_;
	const LightObject* parentLight_;
	std::shared_ptr<Model> model_;
	std::vector<std::unique_ptr<Particle>> particles_;
	Random::RandomNumberGenerator rnd_;
	EmitShape emitShapeType_; 
	int emitTimer_ = 0;

	//OOBB
	Vector3 size_;
	//Sphere
	float radius_;


	
};
