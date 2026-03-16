#pragma once

#include <memory>
#include <list>
#include <vector>

#include "Ghosts.h"
#include "CollisionSystem.h"
#include "MonsterParticle.h"

class Camera;
class GhostsManager {
public:
	void Initialize();
	void Update();

	void Emit(const std::vector<std::vector<bool>>&  emit);
	void SetCamera(const Camera* camera) { camera_ = camera; }
	void SetColliderSystem(CollisionSystem* collisionSystem) { collisionSystem_ = collisionSystem; }
	bool HasBats() const { return !ghostsManager_.empty(); }
	std::list<std::unique_ptr<Ghosts>>& GetBatsGroups() { return ghostsManager_; }

private:
	const Camera* camera_;
	CollisionSystem* collisionSystem_;
	std::list<std::unique_ptr<Ghosts>> ghostsManager_;
	std::unique_ptr<MonsterParticle> monsterParticle_;
};