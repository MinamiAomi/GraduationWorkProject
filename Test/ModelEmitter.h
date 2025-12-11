#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"

enum EmitShape {
	kSphere,
	kBox
};

class ModelEmitter
{
public:
	void Initialize(EmitShape shape);
	void Update();
	void DebugDraw();

	void SetColor(Vector3 color) {
		color_ = color;
	};
	void SetSize(const Vector3& size) { size_ = size; }
	void SetRadius(float radius) { radius_ = radius; }

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
	void SetParent(Transform* parent) { transform_.SetParent(parent); }
public:
	float minSpeed_ = 0.05f;
	float maxSpeed_ = 0.1f;
	Vector3 minAngularVelocity_ = { -0.05f, -0.05f, -0.05f };
	Vector3 maxAngularVelocity_ = { 0.05f,  0.05f,  0.05f };
	float minScaleDecay_ = 0.01f;
	float maxScaleDecay_ = 0.03f;
	uint32_t emitInterval_ = 1;
	float minScale_ = 1.0f;
	float maxScale_ = 1.0f;
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
	std::shared_ptr<Model> model_;
	std::vector<std::unique_ptr<Particle>> particles_;
	Vector3 color_ = Vector3::one;
	Random::RandomNumberGenerator rnd_;
	EmitShape emitShapeType_; 
	uint32_t emitTimer_ = 0;

	//OOBB
	Vector3 size_;
	//Sphere
	float radius_;


	
};

