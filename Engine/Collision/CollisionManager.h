///
/// コリジョンマネージャー
/// 

#pragma once

#include <vector>

#include "Math/MathUtils.h"
#include "Collider.h"

class CollisionManager {
public:
    static CollisionManager* GetInstance();

    void AddCollider(Collider* collider);
    void RemoveCollider(Collider* collider);
    void ClearCollider() { colliders_.clear(); }

    /// <summary>
    /// 衝突をチェック
    /// </summary>
    void CheckCollision();

    /// <summary>
    /// レイキャスト
    /// </summary>
    /// <param name="origin">原点</param>
    /// <param name="diff">ベクトル</param>
    /// <param name="mask">マスク</param>
    /// <param name="nearest">衝突情報</param>
    /// <returns>ヒット有無</returns>
    bool RayCast(const Vector3& origin, const Vector3& diff, uint32_t mask, RayCastInfo* nearest);

private:
    CollisionManager() = default;
    ~CollisionManager() = default;
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;
    CollisionManager(CollisionManager&&) = delete;
    CollisionManager& operator=(CollisionManager&&) = delete;

    std::vector<Collider*> colliders_;
};