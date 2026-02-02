#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"
#include "Collider.h"
#include "BatsParticles.h"

class Camera;

class Bats
{
public:

	static const float batsFarLocate;

	struct Bat {
		ModelInstance modelInstance_;
		std::shared_ptr<Skeleton> skeleton_;
		Transform transform_;
		Transform goalTransform_;
		std::shared_ptr<SphereCollider> collider_;
		BatsParticles particles_;
		float animationTime_;
		bool isUp_;
		float sideStepTime_;
		float hp_;
		bool isDead_;
	};

	Bats(const std::vector<std::vector<bool>>& data, const Camera& camera);
	void Update();
	void DebugDraw();

	void SetRadius(float radius) { radius_ = radius; }

	void SetCamera(const Camera* camera) { camera_ = camera; }

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
	void SetParent(Transform* parent) { transform_.SetParent(parent); }

	bool IsActive() { return isActive_; }

	std::vector<std::unique_ptr<Bat>>& GetBats() { return bats_; }

public:
	std::shared_ptr<Material> material_;
	std::shared_ptr<class AnimationAsset> animation_;
private:
	void Emit(const Vector3& goalPos);
private:
	
	Transform transform_;
	std::shared_ptr<Model> model_;
	std::vector<std::unique_ptr<Bat>> bats_;
	Random::RandomNumberGenerator rnd_;
	const Camera* camera_;
	//Sphere
	float radius_;

	bool isActive_ = false;

};

