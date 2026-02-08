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

const Vector3& BatsManager::GetRandomBatPosition() const
{
    // 全グループの中から、生存しているバットのポインタを一時的に集める
    std::vector<Bats::Bat*> livingBats;

    for (auto& group : batsManager_) {
        for (auto& bat : group->GetBats()) {
            // 生存しており、かつ有効な個体かチェック
            if (bat && !bat->isDead_) {
                livingBats.push_back(bat.get());
            }
        }
    }

    // 生存している個体がいない場合は、安全のためにゼロベクトルなどを返す
    // (戻り値が参照型なので、staticな変数を返すのが一般的です)
    if (livingBats.empty()) {
        static const Vector3 zero = Vector3::zero;
        return zero;
    }

    // ランダムに1つ選ぶ
    // Batsクラスで使っているrnd_があればそれを利用したいところですが、
    // ここでは標準的なランダム、あるいは独自のRandomクラスを想定します
    int randomIndex = rand() % livingBats.size();

    return livingBats[randomIndex]->transform_.translate;
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
