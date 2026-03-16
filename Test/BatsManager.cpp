#include "BatsManager.h"
#include "Trolley.h"

void BatsManager::Initialize()
{
	batsManager_.clear();
   // monsterParticle_ = std::make_unique<MonsterParticle>();
   // monsterParticle_->Initialize();
}

void BatsManager::Update()
{  
   // monsterParticle_->Update();
    uint32_t totalBatCount = 0;
    for (auto it = batsManager_.begin(); it != batsManager_.end(); ) {
        (*it)->Update();

        if (!(*it)->IsActive()) {
            it = batsManager_.erase(it);
        }
        else {
            for (auto& bat : (*it)->GetBats()) {
                // HPが区切りを越えたタイミングでパーティクル発生
                if (bat->GetRatio()) {
                    //monsterParticle_->EmitBatParticle(bat->transform_.translate); // 関数名・引数は要変更
                }
            }

            totalBatCount += static_cast<uint32_t>((*it)->GetBats().size());
            ++it;
        }
    }
    Trolley::GetInstance()->batsNum_ = totalBatCount;
}

void BatsManager::Emit(const std::vector<std::vector<bool>>& emit)
{

    std::unique_ptr<Bats> bats = std::make_unique<Bats>(emit, *camera_);

    for (auto& bat : bats->GetBats()) {
        if (bat->collider_) {
            collisionSystem_->RegisterCollider(bat->collider_);
        }
    }
    
    batsManager_.push_back(std::move(bats));
}
