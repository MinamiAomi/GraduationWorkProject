#include "BatsManager.h"
#include "Trolley.h"

void BatsManager::Initialize()
{
	batsManager_.clear();
}

void BatsManager::Update()
{
    uint32_t totalBatCount = 0; // 合計カウント用の変数

    for (auto it = batsManager_.begin(); it != batsManager_.end(); ) {
        // 更新処理
        (*it)->Update();

        // アクティブチェック
        if (!(*it)->IsActive()) {
            it = batsManager_.erase(it);
        }
        else {
            // アクティブなグループ内のコウモリの数を加算
            // Batsクラスに GetBats() がある前提で、その size() を足します
            totalBatCount += static_cast<uint32_t>((*it)->GetBats().size());
            ++it;
        }
    }

    // カウントした総数を Trolley に代入
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
