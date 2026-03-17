#include "GhostsManager.h"
#include "Trolley.h"

void GhostsManager::Initialize()
{
	ghostsManager_.clear();
	monsterParticle_ = std::make_unique<MonsterParticle>();
	monsterParticle_->Initialize();
}

void GhostsManager::Update()
{

	monsterParticle_->Update();
	for (auto it = ghostsManager_.begin(); it != ghostsManager_.end(); ) {
		(*it)->Update();
		if (!(*it)->IsActive()) {
			it = ghostsManager_.erase(it);
		}
		else {
			for (auto& ghost : (*it)->GetGhosts()) {
				if (ghost->GetRatio()) {
					monsterParticle_->EmitGhostParticle(ghost->transform_.translate);
				}
			}
			++it;
		}
	}
}

void GhostsManager::Emit(const std::vector<std::vector<bool>>& emit)
{

	std::unique_ptr<Ghosts> bats = std::make_unique<Ghosts>(emit, *camera_);
	bats->SetRailCameraSystem(railCameraSystem_);
	for (auto& bat : bats->GetGhosts()) {
		if (bat->collider_) {
			collisionSystem_->RegisterCollider(bat->collider_);
		}
	}

	ghostsManager_.push_back(std::move(bats));
}
