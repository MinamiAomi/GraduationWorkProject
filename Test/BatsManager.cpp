#include "BatsManager.h"

void BatsManager::Initialize()
{
	batsManager_.clear();
}

void BatsManager::Update()
{
    for (auto it = batsManager_.begin(); it != batsManager_.end(); ) {
        // 更新処理
        (*it)->Update();

        // アクティブチェック
        if (!(*it)->IsActive()) {
            // 削除して次の要素のイテレータを取得
            it = batsManager_.erase(it);
        }
        else {
            // 削除しない場合は次へ進める
            ++it;
        }
    }
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
