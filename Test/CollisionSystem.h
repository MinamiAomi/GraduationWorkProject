#pragma once
#include "Collider.h"
#include <list>
#include <memory>


class CollisionSystem {
public:

    static bool isCollisionDebugDraw;

    void Initialize();

    void RegisterCollider(std::shared_ptr<Collider> collider) {
        colliders.push_back(collider);
    }

    // 衝突判定を実行する関数
    void CheckCollisions();

    // 2つのColliderが衝突しているか判定する関数
    bool AreColliding(Collider& a,Collider& b);

    void SetIsActive(bool flag) { isActive_ = flag; }
private:
    bool isActive_;

    std::list<std::weak_ptr<Collider>> colliders;
};