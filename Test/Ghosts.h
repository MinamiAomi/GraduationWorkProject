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
#include "GhostsParticles.h"

class Camera;

class Ghosts
{
public:

	inline static float ghostsFarLocate = 12.0f;

	inline static int ghostAttackFrame = 60 * 15;
	inline static float ghostAttackDamage = 1.0f;

	struct Ghost {
		ModelInstance modelInstance_;
		Transform transform_;
		Transform goalTransform_;
		std::shared_ptr<SphereCollider> collider_;
		GhostsParticles particles_;
		bool isUp_;
		float sideStepTime_;
		float goalT;
		float hp_;
		bool isDead_;
	};

	Ghosts(const std::vector<std::vector<bool>>& data, const Camera& camera);
	void Update();
	void DebugDraw();
	static void Debug();

	void SetRadius(float radius) { radius_ = radius; }

	void SetCamera(const Camera* camera) { camera_ = camera; }

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
	void SetParent(Transform* parent) { transform_.SetParent(parent); }

	bool IsActive() { return isActive_; }

	std::list<std::shared_ptr<Ghost>>& GetGhosts() { return ghost_; }

private:
	void Emit(const Vector3& goalPos);
	std::shared_ptr<Material> material_;
	Vector3 goalColor_ = { 1.0f,0.0f,0.0f };
private:
	const uint32_t kSEMax = 4;
	int attackTime_ = 0;
	Transform transform_;
	std::shared_ptr<Model> model_;
	std::list<std::shared_ptr<Ghost>> ghost_;
	std::list<std::shared_ptr<AudioSource>> playingAudioSourceList_;
	std::shared_ptr<Sound> spawnSESound_;
    std::shared_ptr<Sound> deathSESound_;
	std::shared_ptr<Sound> explodeSESound_;
	uint32_t seCount_ = 0;
	Random::RandomNumberGenerator rnd_;
	const Camera* camera_;
	//Sphere
	float radius_;

	bool isActive_ = false;
	bool isExploded = false;

};

