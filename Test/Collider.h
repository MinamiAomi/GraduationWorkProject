#pragma once
#include <iostream>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Engine/Graphics/RenderManager.h"

enum class CollisionCategory : uint32_t {
    NONE = 0,
    PLAYER = 1 << 0, // 1 (0b0001)
    LIGHT = 1 << 1, // 2 (0b0010)
    ENEMY = 1 << 2, // 4 (0b0100)
    ITEM = 1 << 3, // 8 (0b1000)
};

inline CollisionCategory operator|(CollisionCategory a, CollisionCategory b) {
    return static_cast<CollisionCategory>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

struct SphereCollider;
struct BoxCollider;
struct OBBCollider;
struct ConeCollider;
struct Collider;

struct Collider {

    // 自分がどのカテゴリに属しているか
    CollisionCategory categoryBits;
    uint32_t maskBits;
    const Transform* parent = nullptr;
    Vector3 center;

private:
    std::vector<Collider*> collidedWith;
public:
    // コンストラクタ
    Collider(CollisionCategory category, uint32_t mask, Vector3 pos)
        : categoryBits(category), maskBits(mask), center(pos) {
    }
    virtual ~Collider() {}

    Vector3 GetWorldCenter() const {
        if (parent) {
            return center * parent->worldMatrix;
        }
        return center;
    }

    // 汎用的な CheckCollision (相手に自分を渡す)
    virtual bool CheckCollision(Collider& other) = 0;

    // 相手の型が判明している場合の実際の判定関数
    virtual bool CheckCollision(SphereCollider& other) = 0;
    virtual bool CheckCollision(BoxCollider& other) = 0;
    virtual bool CheckCollision(OBBCollider& other) = 0;
    virtual bool CheckCollision(ConeCollider& other) = 0;

    virtual void OnCollision(Collider& other) {
        collidedWith.push_back(&other);
    }

    void SetParent(const Transform* transform) {
        parent = transform;
    }

    void ClearCollisionInfo() {
        collidedWith.clear();
    }
    const std::vector<Collider*>& GetCollidedWith() const {
        return collidedWith;
    }
};

struct SphereCollider : public Collider {
    float radius;

    SphereCollider(CollisionCategory category, uint32_t mask, Vector3 pos, float r)
        : Collider(category, mask, pos), radius(r) {
    }

    float GetWorldRadius() const {
        if (parent) {
            Vector3 globalScale = parent->worldMatrix.GetScale();
            float maxScale = std::max({ globalScale.x, globalScale.y, globalScale.z });
            return radius * maxScale;
        }
        return radius;
    }

    bool CheckCollision(Collider& other) override;
    bool CheckCollision(SphereCollider& other) override;
    bool CheckCollision(BoxCollider& other) override;
    bool CheckCollision(OBBCollider& other) override;
    bool CheckCollision(ConeCollider& other) override;
};

struct BoxCollider : public Collider {
    Vector3 size;

    BoxCollider(CollisionCategory category, uint32_t mask, Vector3 pos, Vector3 s)
        : Collider(category, mask, pos), size(s) {
    }
    Vector3 GetWorldSize() const {
        if (parent) {
            Vector3 worldScale = parent->worldMatrix.GetScale();
            return Vector3(
                std::abs(size.x * worldScale.x),
                std::abs(size.y * worldScale.y),
                std::abs(size.z * worldScale.z)
            );
        }
        return size;
    }
    Vector3 GetWorldMin() const {
        return GetWorldCenter() - (size * 0.5f);
    }

    Vector3 GetWorldMax() const {
        return GetWorldCenter() + (size * 0.5f);
    }

    bool CheckCollision(Collider& other) override;
    bool CheckCollision(SphereCollider& other) override;
    bool CheckCollision(BoxCollider& other) override;
    bool CheckCollision(OBBCollider& other) override;
    bool CheckCollision(ConeCollider& other) override;
};

struct OBBCollider : public Collider {
    Vector3 size;           
    Quaternion quaternion; 



    OBBCollider(CollisionCategory category, uint32_t mask, Vector3 pos, Vector3 s, Quaternion rot)
        : Collider(category, mask, pos), size(s), quaternion(rot) {
    }

    Quaternion GetWorldOrientation() const {
        if (parent) {
            Quaternion parentWorldRot = parent->worldMatrix.GetRotate();
            return parentWorldRot * quaternion;
        }
        return quaternion;
    }

    Vector3 GetWorldSize() const {
        if (parent) {
            Vector3 worldScale = parent->worldMatrix.GetScale();
            return Vector3(
                std::abs(size.x * worldScale.x),
                std::abs(size.y * worldScale.y),
                std::abs(size.z * worldScale.z)
            );
        }
        return size;
    }

    bool CheckCollision(Collider& other) override;
    bool CheckCollision(SphereCollider& other) override;
    bool CheckCollision(BoxCollider& other) override;
    bool CheckCollision(OBBCollider& other) override;
    bool CheckCollision(ConeCollider& other) override;
};

struct ConeCollider : public Collider {
    float radius;
    float height;
    Quaternion quaternion; 

    ConeCollider(CollisionCategory category, uint32_t mask, Vector3 pos, float r, float h, Quaternion rot)
        : Collider(category, mask, pos), radius(r), height(h), quaternion(rot) {
    }

    Quaternion GetWorldOrientation() const {
        if (parent) {
            Quaternion parentWorldRot = parent->worldMatrix.GetRotate();
            return parentWorldRot * quaternion;
        }
        return quaternion;
    }

    // 宣言のみ
    bool CheckCollision(Collider& other) override;
    bool CheckCollision(SphereCollider& other) override;
    bool CheckCollision(BoxCollider& other) override;
    bool CheckCollision(OBBCollider& other) override;
    bool CheckCollision(ConeCollider& other) override;
};

#pragma region 当たり判定ヘルパー

inline bool CheckOBBvsOBB(const OBBCollider& a, const OBBCollider& b) {
  
    Vector3 aPos = a.GetWorldCenter();
    Vector3 bPos = b.GetWorldCenter();
    Quaternion aRot = a.GetWorldOrientation();
    Quaternion bRot = b.GetWorldOrientation();

    Vector3 aExtents = a.GetWorldSize() * 0.5f;
    Vector3 bExtents = b.GetWorldSize() * 0.5f;

    Vector3 aAxes[3], bAxes[3];
    aRot.GetAxes(aAxes);
    bRot.GetAxes(bAxes);

    Vector3 dist = bPos - aPos;

    // 以下の15軸について分離軸があるかチェック
    // 1. Aのxyz軸 (3本)
    // 2. Bのxyz軸 (3本)
    // 3. Aの各軸 x Bの各軸 (9本)

    // ※ R_ij = Dot(aAxes[i], bAxes[j]) の絶対値
    // 計算誤差対策で微小値を足しておくのが一般的
    float R[3][3];
    float AbsR[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            R[i][j] = Vector3::Dot(aAxes[i], bAxes[j]);
            AbsR[i][j] = std::abs(R[i][j]) + 1e-6f;
        }
    }

    // 中心間距離のAローカル軸への射影
    float t[3];
    t[0] = Vector3::Dot(dist, aAxes[0]);
    t[1] = Vector3::Dot(dist, aAxes[1]);
    t[2] = Vector3::Dot(dist, aAxes[2]);

    // --- Test 1: Aの各軸 (A0, A1, A2) ---
    for (int i = 0; i < 3; i++) {
        float ra = aExtents[i];
        float rb = bExtents[0] * AbsR[i][0] + bExtents[1] * AbsR[i][1] + bExtents[2] * AbsR[i][2];
        if (std::abs(t[i]) > ra + rb) return false; // 分離している
    }

    // --- Test 2: Bの各軸 (B0, B1, B2) ---
    for (int i = 0; i < 3; i++) {
        float ra = aExtents[0] * AbsR[0][i] + aExtents[1] * AbsR[1][i] + aExtents[2] * AbsR[2][i];
        float rb = bExtents[i];
        float tProj = std::abs(t[0] * R[0][i] + t[1] * R[1][i] + t[2] * R[2][i]);
        if (tProj > ra + rb) return false;
    }

    // --- Test 3: クロス積 (A_i x B_j) ---
    // A0 x B0
    if (std::abs(t[2] * R[1][0] - t[1] * R[2][0]) >
        aExtents[1] * AbsR[2][0] + aExtents[2] * AbsR[1][0] +
        bExtents[1] * AbsR[0][2] + bExtents[2] * AbsR[0][1]) return false;
    // A0 x B1
    if (std::abs(t[2] * R[1][1] - t[1] * R[2][1]) >
        aExtents[1] * AbsR[2][1] + aExtents[2] * AbsR[1][1] +
        bExtents[0] * AbsR[0][2] + bExtents[2] * AbsR[0][0]) return false;
    // A0 x B2
    if (std::abs(t[2] * R[1][2] - t[1] * R[2][2]) >
        aExtents[1] * AbsR[2][2] + aExtents[2] * AbsR[1][2] +
        bExtents[0] * AbsR[0][1] + bExtents[1] * AbsR[0][0]) return false;

    // A1 x B0
    if (std::abs(t[0] * R[2][0] - t[2] * R[0][0]) >
        aExtents[0] * AbsR[2][0] + aExtents[2] * AbsR[0][0] +
        bExtents[1] * AbsR[1][2] + bExtents[2] * AbsR[1][1]) return false;
    // A1 x B1
    if (std::abs(t[0] * R[2][1] - t[2] * R[0][1]) >
        aExtents[0] * AbsR[2][1] + aExtents[2] * AbsR[0][1] +
        bExtents[0] * AbsR[1][2] + bExtents[2] * AbsR[1][0]) return false;
    // A1 x B2
    if (std::abs(t[0] * R[2][2] - t[2] * R[0][2]) >
        aExtents[0] * AbsR[2][2] + aExtents[2] * AbsR[0][2] +
        bExtents[0] * AbsR[1][1] + bExtents[1] * AbsR[1][0]) return false;

    // A2 x B0
    if (std::abs(t[1] * R[0][0] - t[0] * R[1][0]) >
        aExtents[0] * AbsR[1][0] + aExtents[1] * AbsR[0][0] +
        bExtents[1] * AbsR[2][2] + bExtents[2] * AbsR[2][1]) return false;
    // A2 x B1
    if (std::abs(t[1] * R[0][1] - t[0] * R[1][1]) >
        aExtents[0] * AbsR[1][1] + aExtents[1] * AbsR[0][1] +
        bExtents[0] * AbsR[2][2] + bExtents[2] * AbsR[2][0]) return false;
    // A2 x B2
    if (std::abs(t[1] * R[0][2] - t[0] * R[1][2]) >
        aExtents[0] * AbsR[1][2] + aExtents[1] * AbsR[0][2] +
        bExtents[0] * AbsR[2][1] + bExtents[1] * AbsR[2][0]) return false;

    // 全ての軸で重なりがあれば衝突
    return true;
}

inline bool CheckConeVsSphere(const ConeCollider& cone, const SphereCollider& sphere) {

    Vector3 conePos = cone.GetWorldCenter();
    Quaternion coneRot = cone.GetWorldOrientation();
    Vector3 spherePos = sphere.GetWorldCenter();
    float sphereRadius = sphere.GetWorldRadius();

    Vector3 diff = spherePos - conePos;
    Quaternion invRot = coneRot.Conjugate();
    Vector3 localPos = invRot * diff;

    float axisY = localPos.y;
    float axisR = sqrt(localPos.x * localPos.x + localPos.z * localPos.z);
    float coneH = cone.height;
    float coneR = cone.radius;
    float edgeX = coneR;
    float edgeY = -coneH;
    float edgeLenSq = edgeX * edgeX + edgeY * edgeY;
    float pX = axisR;
    float pY = axisY - coneH;
    float t = (pX * edgeX + pY * edgeY) / edgeLenSq;
    t = std::clamp(t, 0.0f, 1.0f);
    float closestX = t * edgeX;
    float closestY = coneH + t * edgeY;

    if (axisY < 0.0f) {
        if (axisR < coneR) { closestY = 0.0f; closestX = axisR; }
        else { closestY = 0.0f; closestX = coneR; }
    }
    else if (axisY > coneH) {
        if (t <= 0.0f && axisR == 0.0f) { closestX = 0.0f; closestY = coneH; }
    }

    bool isInside = false;
    if (axisY >= 0 && axisY <= coneH) {
        float allowedR = (coneH - axisY) * (coneR / coneH);
        if (axisR <= allowedR) isInside = true;
    }
    if (isInside) return true;

    float dX = axisR - closestX;
    float dY = axisY - closestY;
    float distSq = dX * dX + dY * dY;

    return distSq <= (sphereRadius * sphereRadius);
}
#pragma endregion

inline bool SphereCollider::CheckCollision(Collider& other) {
    return other.CheckCollision(*this);
}

inline bool SphereCollider::CheckCollision(SphereCollider& other) {
    Vector3 myPos = GetWorldCenter();
    Vector3 otherPos = other.GetWorldCenter();
    float distSq = (myPos - otherPos).LengthSquare();
    float rSum = GetWorldRadius() + other.GetWorldRadius();
    return distSq <= rSum * rSum;
}
inline bool SphereCollider::CheckCollision(BoxCollider& other) { return other.CheckCollision(*this); }
inline bool SphereCollider::CheckCollision(OBBCollider& other) { return other.CheckCollision(*this); }
inline bool SphereCollider::CheckCollision(ConeCollider& other) { return other.CheckCollision(*this); }

inline bool BoxCollider::CheckCollision(Collider& other) { return other.CheckCollision(*this); }
inline bool BoxCollider::CheckCollision(SphereCollider& other) {
    Vector3 min = GetWorldMin();
    Vector3 max = GetWorldMax();
    Vector3 sPos = other.GetWorldCenter();
    float cx = std::clamp(sPos.x, min.x, max.x);
    float cy = std::clamp(sPos.y, min.y, max.y);
    float cz = std::clamp(sPos.z, min.z, max.z);
    return (Vector3(cx, cy, cz) - sPos).LengthSquare() <= std::pow(other.GetWorldRadius(), 2);
}

inline bool BoxCollider::CheckCollision(BoxCollider& other) {
    Vector3 aMin = GetWorldMin(); Vector3 aMax = GetWorldMax();
    Vector3 bMin = other.GetWorldMin(); Vector3 bMax = other.GetWorldMax();
    return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
        (aMin.y <= bMax.y && aMax.y >= bMin.y) &&
        (aMin.z <= bMax.z && aMax.z >= bMin.z);
}

inline bool BoxCollider::CheckCollision(OBBCollider& other) {
    Quaternion id = Quaternion::identity;
    OBBCollider temp(CollisionCategory::NONE, 0, GetWorldCenter(), GetWorldSize(), id);
    return CheckOBBvsOBB(temp, other);
}
inline bool BoxCollider::CheckCollision(ConeCollider& other) { return other.CheckCollision(*this); }

inline bool OBBCollider::CheckCollision(Collider& other) {
    return other.CheckCollision(*this);
}

inline bool OBBCollider::CheckCollision(SphereCollider& other) {
    Vector3 myPos = GetWorldCenter();
    Quaternion myRot = GetWorldOrientation();
    Vector3 sPos = other.GetWorldCenter();

    Vector3 diff = sPos - myPos;
    Quaternion invRot = myRot.Conjugate(); 
    Vector3 localS = invRot * diff;

    Vector3 extents = GetWorldSize() * 0.5f;

    Vector3 boxMin = -extents;
    Vector3 boxMax = extents;

    float cx = std::clamp(localS.x, boxMin.x, boxMax.x);
    float cy = std::clamp(localS.y, boxMin.y, boxMax.y);
    float cz = std::clamp(localS.z, boxMin.z, boxMax.z);

    return (Vector3(cx, cy, cz) - localS).LengthSquare() <= std::pow(other.GetWorldRadius(), 2);
}

inline bool OBBCollider::CheckCollision(BoxCollider& other) { return other.CheckCollision(*this); }
inline bool OBBCollider::CheckCollision(OBBCollider& other) { return CheckOBBvsOBB(*this, other); }
inline bool OBBCollider::CheckCollision(ConeCollider& other) { return other.CheckCollision(*this); }

inline bool ConeCollider::CheckCollision(Collider& other) { return other.CheckCollision(*this); }

inline bool ConeCollider::CheckCollision(SphereCollider& other) {
    return CheckConeVsSphere(*this, other);
}

inline bool ConeCollider::CheckCollision(BoxCollider& other) {
    // Boxを包含する球の判定で近似
    float boxRadius = other.GetWorldSize().Length() * 0.5f; 
    SphereCollider tempSphere(CollisionCategory::NONE, 0, other.GetWorldCenter(), boxRadius);
    return CheckConeVsSphere(*this, tempSphere);
}

inline bool ConeCollider::CheckCollision(OBBCollider& other) {
    // OBBを包含する球で近似
    float obbRadius = other.GetWorldSize().Length() * 0.5f;
    SphereCollider tempSphere(CollisionCategory::NONE, 0, other.GetWorldCenter(), obbRadius);
    return CheckConeVsSphere(*this, tempSphere);
}

inline bool ConeCollider::CheckCollision(ConeCollider& other) {
    //このあたり判定はしないで
    other;
    return false;
}