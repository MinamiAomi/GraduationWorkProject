#pragma once

#include <memory>
#include <list>
#include <vector>

#include "Bats.h"
#include "MonsterParticle.h"
#include "CollisionSystem.h"

class Camera;
class BatsManager {
public:
	void Initialize();
	void Update();

	void Emit(const std::vector<std::vector<bool>>&  emit);
	void SetCamera(const Camera* camera) { camera_ = camera; }
	void SetColliderSystem(CollisionSystem* collisionSystem) { collisionSystem_ = collisionSystem; }
	bool HasBats() const { return !batsManager_.empty(); }
	std::list<std::unique_ptr<Bats>>& GetBatsGroups() { return batsManager_; }

private:
	const Camera* camera_;
	CollisionSystem* collisionSystem_;
	std::list<std::unique_ptr<Bats>> batsManager_;
	std::unique_ptr<MonsterParticle> monsterParticle_;
};