#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"
#include "Collider.h"

class Camera;

class Bats
{
public:

	static const float batsFarLocate;

	Bats(const std::vector<std::vector<bool>>& data,const Camera& camera);
	void Update();
	void DebugDraw();

	void SetRadius(float radius) { radius_ = radius; }

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
	void SetParent(Transform* parent) { transform_.SetParent(parent); }
public:
	std::shared_ptr<Material> material_;
private:
	void Emit(const Vector3& goalPos);
private:
	struct Bat {
		ModelInstance modelInstance_;
		Transform transform_;
		Transform goalTransform_;
		std::shared_ptr<SphereCollider> collider_;

	};


	Transform transform_;
	std::shared_ptr<Model> model_;
	std::vector<std::unique_ptr<Bat>> bats_;
	Random::RandomNumberGenerator rnd_;
	const Camera* camera_;
	//Sphere
	float radius_;
	
};

