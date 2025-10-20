#include "CollisionManager.h"

CollisionManager* CollisionManager::GetInstance() {
    static CollisionManager instance;
    return &instance;
}

void CollisionManager::AddCollider(Collider* collider) { 
    colliders_.emplace_back(collider); 
}

void CollisionManager::RemoveCollider(Collider* collider) {
    auto iter = std::find(colliders_.begin(), colliders_.end() , collider);
    if (iter != colliders_.end()) {
        colliders_.erase(iter);
    }
}

void CollisionManager::CheckCollision() {

    auto iter1 = colliders_.begin();
    for (; iter1 != colliders_.end(); ++iter1) {
        Collider* collider1 = *iter1;
        // アクティブじゃなければ通さない
        if (!collider1->IsActive()) { continue; }

        auto iter2 = iter1;
        ++iter2;
        for (; iter2 != colliders_.end(); ++iter2) {
            Collider* collider2 = *iter2;
            // アクティブじゃなければ通さない
            if (!collider2->IsActive()) { continue; }

            CollisionInfo collisionInfo1;      
            if (collider1->IsCollision(collider2, collisionInfo1)) {
                // 衝突情報を反転
                CollisionInfo collisionInfo2 = collisionInfo1;
                collisionInfo2.gameObject = collider1->GetGameObject();
                collisionInfo2.normal = -collisionInfo1.normal;
                // Stayを呼び出す
                collider1->OnCollision(collisionInfo1);
                collider2->OnCollision(collisionInfo2);
            }
        }
    }

}

bool CollisionManager::RayCast(const Vector3& origin, const Vector3& diff, uint32_t mask, RayCastInfo* nearest) {
    
    RayCastInfo tmpNearest{};
    tmpNearest.nearest = 1.1f;

    for (auto collider : colliders_) {
        RayCastInfo info{};
        info.nearest = FLT_MAX;
        if (collider->RayCast(origin, diff, mask, info)) {
            if (info.nearest < tmpNearest.nearest) {
                tmpNearest = info;
            }
        }   
    }
    if (tmpNearest.nearest > 1.0f) {
        return false;
    }
    if (nearest) {
        *nearest = tmpNearest;
    }

    return true;
}
