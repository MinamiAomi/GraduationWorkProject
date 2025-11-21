#include "CollisionSystem.h"

void CollisionSystem::CheckCollisions()
{
    //全Colliderの衝突情報をクリア
    for (const auto& weak_ptr : colliders) {
        if (auto ptr = weak_ptr.lock()) { // 生存確認
            ptr->ClearCollisionInfo();
        }
    }

    for (size_t i = 0; i < colliders.size(); ++i) {
        for (size_t j = i + 1; j < colliders.size(); ++j) {

            std::shared_ptr<Collider> a_ptr = colliders[i].lock();
            std::shared_ptr<Collider> b_ptr = colliders[j].lock();

            // 2つのColliderで衝突判定を実行
            if (a_ptr && b_ptr) {
                if (AreColliding(*a_ptr, *b_ptr)) {

                    //情報をリストに貯める
                    a_ptr->OnCollision(*b_ptr);
                    b_ptr->OnCollision(*a_ptr);
                }
            }
        }
    }

    colliders.erase(
        std::remove_if(colliders.begin(), colliders.end(),
            [](const std::weak_ptr<Collider>& wp) {
                return wp.expired(); // 参照先が失効しているか？
            }),
        colliders.end()
    );

}

bool CollisionSystem::AreColliding(Collider& a, Collider& b)
{
    bool canCollide = (a.maskBits & static_cast<uint32_t>(b.categoryBits)) != 0 &&
        (b.maskBits & static_cast<uint32_t>(a.categoryBits)) != 0;

    if (!canCollide) {
        return false;
    }

    bool result = a.CheckCollision(b);

    return result;
}
