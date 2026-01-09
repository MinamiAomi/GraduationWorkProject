#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"
#include "LightObject.h"

enum EmitShape {
	kSphere,
	kBox
};

class PowerEmitter
{
public:
	void Initialize(EmitShape shape, const LightObject* parentLight);
	void Update();
	void DebugDraw();

	void SetColor(Vector3 color) {
		material_->albedo = color;
	};
	void SetSize(const Vector3& size) { size_ = size; }
	void SetRadius(float radius) { radius_ = radius; }

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
public:
	std::shared_ptr<Material> material_;
	float minSpeed_ = 0.003f;
	float maxSpeed_ = 0.002f;
	Vector3 minAngularVelocity_ = { -0.05f, -0.05f, -0.05f };
	Vector3 maxAngularVelocity_ = { 0.05f,  0.05f,  0.05f };
	float minScaleDecay_ = 0.01f;
	float maxScaleDecay_ = 0.03f;
	uint32_t emitInterval_ = 1;
	float minScale_ = 0.5f;
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
	};


	Transform transform_;
	const LightObject* parentLight_;
	std::shared_ptr<Model> model_;
	std::vector<std::unique_ptr<Particle>> particles_;
	Random::RandomNumberGenerator rnd_;
	EmitShape emitShapeType_; 
	uint32_t emitTimer_ = 0;

	//OOBB
	Vector3 size_;
	//Sphere
	float radius_;
	
};

