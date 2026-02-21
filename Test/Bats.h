#pragma once

#include <vector>
#include <list>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"
#include "Collider.h"
#include "BatsParticles.h"
#include "BatsBullets.h"
#include "Audio/AudioSource.h"

class Camera;

class Bats
{
public:

	inline static float batsFarLocate = 12.0f;
	inline static float damage = 0.2f;
	inline static float heal = 0.1f;

	struct Bat {
		ModelInstance modelInstance_;
		std::shared_ptr<Skeleton> skeleton_;
		Transform transform_;
		Transform goalTransform_;
		std::shared_ptr<SphereCollider> collider_;
		BatsParticles particles_;
		//BatsBullets bullets_;
		float animationTime_;
		float goalT;
		bool isUp_;
		float sideStepTime_;
		float hp_;
		bool isDead_;
	};

	Bats(const std::vector<std::vector<bool>>& data, const Camera& camera);
	void Update();
	void DebugDraw();
#ifdef _DEBUG
	static void Debug();
#endif // _DEBUG


	void SetRadius(float radius) { radius_ = radius; }

	void SetCamera(const Camera* camera) { camera_ = camera; }

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
	void SetParent(Transform* parent) { transform_.SetParent(parent); }

	bool IsActive() { return isActive_; }

	std::list<std::shared_ptr<Bat>>& GetBats() { return bats_; }

public:
	std::shared_ptr<class AnimationAsset> animation_;
private:
	void Emit(const Vector3& goalPos);
private:
	const uint32_t kSEMax = 10;

	Transform transform_;
	std::shared_ptr<Model> model_;
	std::list<std::shared_ptr<Bat>> bats_;
	std::list<std::shared_ptr<AudioSource>> playingAudioSourceList_;
	std::shared_ptr<Sound> spawnSESound_;
    std::shared_ptr<Sound> deathSESound_;
	uint32_t seCount_ = 0;
	Random::RandomNumberGenerator rnd_;
	const Camera* camera_;
	//Sphere
	float radius_;

	bool isActive_ = false;

};

