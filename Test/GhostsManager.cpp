#include "GhostsManager.h"
#include "Trolley.h"

void GhostsManager::Initialize()
{
    ghostsManager_.clear();
}

void GhostsManager::Update()
{

    for (auto it = ghostsManager_.begin(); it != ghostsManager_.end(); ) {
        // 更新処理
        (*it)->Update();

        // アクティブチェック
        if (!(*it)->IsActive()) {
            it = ghostsManager_.erase(it);
        }
        else {
            // アクティブなグループ内のコウモリの数を加算
            // Batsクラスに GetBats() がある前提で、その size() を足します
            ++it;
        }
    }

}

void GhostsManager::Emit(const std::vector<std::vector<bool>>& emit)
{

    std::unique_ptr<Ghosts> bats = std::make_unique<Ghosts>(emit, *camera_);

    for (auto& bat : bats->GetGhosts()) {
        if (bat->collider_) {
            collisionSystem_->RegisterCollider(bat->collider_);
        }
    }
    
    ghostsManager_.push_back(std::move(bats));
}
