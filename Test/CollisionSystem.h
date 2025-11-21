#pragma once
#include "Collider.h"
#include <vector>
#include <memory>

class CollisionSystem {
public:

    void RegisterCollider(std::shared_ptr<Collider> collider) {
        colliders.push_back(collider);
    }

    // 衝突判定を実行する関数
    void CheckCollisions();

    // 2つのColliderが衝突しているか判定する関数
    bool AreColliding(Collider& a,Collider& b);

private:
    std::vector<std::weak_ptr<Collider>> colliders;
};