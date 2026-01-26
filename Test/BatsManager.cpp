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
    batsManager_.push_back(std::make_unique<Bats>(emit,*camera_));
}
