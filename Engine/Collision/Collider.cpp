#include "Collider.h"

#include <array>

#include "CollisionManager.h"

#include <limits>

namespace {

    std::vector<Vector3> GetVertices(const Math::OBB& obb) {
        Vector3 halfSize = obb.size * 0.5f;

        std::vector<Vector3> vertices(8);

        vertices[0] = { -halfSize.x, -halfSize.y, -halfSize.z };   // 左下前
        vertices[1] = { -halfSize.x,  halfSize.y, -halfSize.z };   // 左上前
        vertices[2] = { halfSize.x,  halfSize.y, -halfSize.z };   // 右上前
        vertices[3] = { halfSize.x, -halfSize.y, -halfSize.z };   // 右下前
        vertices[4] = { -halfSize.x, -halfSize.y,  halfSize.z };   // 左下奥 
        vertices[5] = { -halfSize.x,  halfSize.y,  halfSize.z };   // 左上奥
        vertices[6] = { halfSize.x,  halfSize.y,  halfSize.z };   // 右上奥
        vertices[7] = { halfSize.x, -halfSize.y,  halfSize.z };   // 右下奥

        Matrix4x4 obbWorldMatrix =
            Matrix4x4().SetXAxis(obb.orientations[0]).SetYAxis(obb.orientations[1]).SetZAxis(obb.orientations[2]).SetTranslate(obb.center);
        for (size_t i = 0; i < vertices.size(); ++i) {
            vertices[i] = vertices[i] * obbWorldMatrix;
        }

        return vertices;
    }

    Vector2 Projection(const std::vector<Vector3>& vertices, const Vector3& axis) {
        Vector2 minmax(Dot(axis, vertices[0]));
        for (size_t i = 1; i < vertices.size(); ++i) {
            float dot = Dot(axis, vertices[i]);
            minmax.x = std::min(dot, minmax.x);
            minmax.y = std::max(dot, minmax.y);
        }
        return minmax;
    }

    float GetOverlap(const Vector2& minmax1, const Vector2& minmax2) {
        float range1 = minmax1.y - minmax1.x;
        float range2 = minmax2.y - minmax2.x;
        float maxOverlap = std::max(minmax1.y, minmax2.y) - std::min(minmax1.x, minmax2.x);
        return range1 + range2 - maxOverlap;
    }
}

Collider::Collider() {
    CollisionManager::GetInstance()->AddCollider(this);
}

Collider::~Collider() {
    CollisionManager::GetInstance()->RemoveCollider(this);
}

void Collider::OnCollision(const CollisionInfo& collisionInfo) {
    if (callback_) {
        callback_(collisionInfo);
    }
}

bool Collider::CanCollision(Collider* other) const {
    return (this->collisionAttribute_ & other->collisionMask_) && (other->collisionAttribute_ & this->collisionMask_);
}

bool Collider::CanCollision(uint32_t mask) const {
    return (this->collisionAttribute_ & mask);
}

bool SphereCollider::IsCollision(Collider* other, CollisionInfo& collisionInfo) {
    if (CanCollision(other)) {
        return  other->IsCollision(this, collisionInfo);
    }
    return false;
}

bool SphereCollider::IsCollision(SphereCollider* other, CollisionInfo& collisionInfo) {
    // 差分ベクトルを求める
    Vector3 diff = other->sphere_.center - this->sphere_.center;
    float hitRange = this->sphere_.radius + other->sphere_.radius;
    if (diff.LengthSquare() > hitRange * hitRange) {
        return false;
    }

    // 衝突情報を格納していく
    float length = diff.Length();
    collisionInfo.gameObject = this->GetGameObject();
    if (length != 0.0f) {
        collisionInfo.normal = diff / length;
    }
    else {
        collisionInfo.normal = Vector3::zero;
    }
    collisionInfo.depth = hitRange - length;
    return true;
}

bool SphereCollider::IsCollision(BoxCollider* other, CollisionInfo& collisionInfo) {
    Math::Sphere& sphere = this->sphere_;
    Math::OBB& obb = other->obb_;
    // obbのローカル空間で衝突判定を行う
    Matrix4x4 obbRotateMatrix = Matrix4x4().SetXAxis(obb.orientations[0]).SetYAxis(obb.orientations[1]).SetZAxis(obb.orientations[2]);
    Matrix4x4 obbWorldInverse = Matrix4x4::MakeAffineInverse(obbRotateMatrix, obb.center);
    Vector3 centerInOBBLocal = sphere.center * obbWorldInverse;
    Vector3 halfSize = obb.size * 0.5f;

    Vector3 point = {
          std::clamp(centerInOBBLocal.x, -halfSize.x, halfSize.x),
          std::clamp(centerInOBBLocal.y, -halfSize.y, halfSize.y),
          std::clamp(centerInOBBLocal.z, -halfSize.z, halfSize.z) };
    Vector3 diff = point - centerInOBBLocal;

    if (diff.LengthSquare() > sphere.radius * sphere.radius) {
        return false;
    }

    // 衝突情報を格納していく
    float length = diff.Length();
    collisionInfo.gameObject = this->GetGameObject();
    if (length != 0.0f) {
        collisionInfo.normal = (diff / length) * obbRotateMatrix;
    }
    else {
        collisionInfo.normal = Vector3::zero;
    }
    collisionInfo.depth = sphere.radius - length;
    return true;
}

bool SphereCollider::RayCast(const Vector3& origin, const Vector3& diff, uint32_t mask, RayCastInfo& nearest) {
    if (!CanCollision(mask)) { return false; }

    Vector3 direction = diff.Normalized();
    float distance = Dot(sphere_.center - origin, direction);
    distance = std::max(distance, 0.0f);
    Vector3 closestPoint = origin + distance * direction;

    if ((closestPoint - sphere_.center).LengthSquare() > sphere_.radius * sphere_.radius) {
        return false;
    }
    nearest.nearest = distance / diff.Length();
    return true;
}

bool BoxCollider::IsCollision(Collider* other, CollisionInfo& collisionInfo) {
    if (CanCollision(other)) {
        return  other->IsCollision(this, collisionInfo);
    }
    return false;
}

bool BoxCollider::IsCollision(SphereCollider* other, CollisionInfo& collisionInfo) {
    Math::Sphere& sphere = other->sphere_;
    Math::OBB& obb = this->obb_;
    // obbのローカル空間で衝突判定を行う
    Matrix4x4 obbRotateMatrix = Matrix4x4().SetXAxis(obb.orientations[0]).SetYAxis(obb.orientations[1]).SetZAxis(obb.orientations[2]);
    Matrix4x4 obbWorldInverse = Matrix4x4::MakeAffineInverse(obbRotateMatrix, obb.center);
    Vector3 centerInOBBLocal = sphere.center * obbWorldInverse;
    Vector3 halfSize = obb.size * 0.5f;

    Vector3 point = {
          std::clamp(centerInOBBLocal.x, -halfSize.x, halfSize.x),
          std::clamp(centerInOBBLocal.y, -halfSize.y, halfSize.y),
          std::clamp(centerInOBBLocal.z, -halfSize.z, halfSize.z) };
    Vector3 diff = centerInOBBLocal - point;

    if (diff.LengthSquare() > sphere.radius * sphere.radius) {
        return false;
    }

    // 衝突情報を格納していく
    float length = diff.Length();
    collisionInfo.gameObject = this->GetGameObject();
    if (length != 0.0f) {
        collisionInfo.normal = (diff / length) * obbRotateMatrix;
    }
    else {
        collisionInfo.normal = Vector3::zero;
    }
    collisionInfo.depth = sphere.radius - length;
    return true;
}

bool BoxCollider::IsCollision(BoxCollider* other, CollisionInfo& collisionInfo) {
    auto vertices1 = GetVertices(this->obb_);
    auto vertices2 = GetVertices(other->obb_);

    Vector3 axes[] = {
        this->obb_.orientations[0],
        this->obb_.orientations[1],
        this->obb_.orientations[2],

        other->obb_.orientations[0],
        other->obb_.orientations[1],
        other->obb_.orientations[2],

        Cross(this->obb_.orientations[0], other->obb_.orientations[0]).Normalized(),
        Cross(this->obb_.orientations[0], other->obb_.orientations[1]).Normalized(),
        Cross(this->obb_.orientations[0], other->obb_.orientations[2]).Normalized(),

        Cross(this->obb_.orientations[1], other->obb_.orientations[0]).Normalized(),
        Cross(this->obb_.orientations[1], other->obb_.orientations[1]).Normalized(),
        Cross(this->obb_.orientations[1], other->obb_.orientations[2]).Normalized(),

        Cross(this->obb_.orientations[2], other->obb_.orientations[0]).Normalized(),
        Cross(this->obb_.orientations[2], other->obb_.orientations[1]).Normalized(),
        Cross(this->obb_.orientations[2], other->obb_.orientations[2]).Normalized(),
    };

    float minOverlap = FLT_MAX;
    Vector3 minOverlapAxis = {};

    // 分離軸判定関数
    auto IsSeparateAxis = [&](const Vector3& axis) {
        if (axis == Vector3::zero) { return false; }
        Vector2 minmax1 = Projection(vertices1, axis);
        Vector2 minmax2 = Projection(vertices2, axis);

        // 分離軸である
        if (!(minmax1.x <= minmax2.y && minmax1.y >= minmax2.x)) {
            return true;
        }

        float overlap = GetOverlap(minmax1, minmax2);

        if (overlap < minOverlap) {
            minOverlapAxis = axis;
            minOverlap = overlap;
        }

        return false;
    };

    for (auto& axis : axes) {
        if (std::isnan(axis.x) || std::isnan(axis.y) || std::isnan(axis.z)) { continue; }
        if (IsSeparateAxis(axis)) { return false; }
    }

    // 衝突情報を格納していく
    collisionInfo.gameObject = this->GetGameObject();
    collisionInfo.normal = minOverlapAxis.Normalized();
    if (Dot(other->obb_.center - this->obb_.center, collisionInfo.normal) < 0.0f) {
        collisionInfo.normal *= -1;
    }
    collisionInfo.depth = minOverlap;
    return true;
}

bool BoxCollider::RayCast(const Vector3& origin, const Vector3& diff, uint32_t mask, RayCastInfo& nearest) {
    if (!CanCollision(mask)) { return false; }

    Math::OBB& obb = this->obb_;
    // obbのローカル空間で衝突判定を行う
    Matrix4x4 obbRotateMatrix = Matrix4x4().SetXAxis(obb.orientations[0]).SetYAxis(obb.orientations[1]).SetZAxis(obb.orientations[2]);
    Matrix4x4 obbWorldInverse = Matrix4x4::MakeAffineInverse(obbRotateMatrix, obb.center);
    Vector3 halfSize = obb.size * 0.5f;
    Math::AABB aabbInOBBLocal{ -halfSize, halfSize };

    Vector3 originInOBBLocal = origin * obbWorldInverse;
    Vector3 diffInOBBLocal = ((origin + diff) * obbWorldInverse) - originInOBBLocal;


    float tXMin = (aabbInOBBLocal.min.x - originInOBBLocal.x) / diffInOBBLocal.x;
    float tXMax = (aabbInOBBLocal.max.x - originInOBBLocal.x) / diffInOBBLocal.x;
    float tYMin = (aabbInOBBLocal.min.y - originInOBBLocal.y) / diffInOBBLocal.y;
    float tYMax = (aabbInOBBLocal.max.y - originInOBBLocal.y) / diffInOBBLocal.y;
    float tZMin = (aabbInOBBLocal.min.z - originInOBBLocal.z) / diffInOBBLocal.z;
    float tZMax = (aabbInOBBLocal.max.z - originInOBBLocal.z) / diffInOBBLocal.z;

    // 軸に平行かついずれかがmin/maxと一致
    if (std::isnan(tXMin) || std::isnan(tXMax) ||
        std::isnan(tYMin) || std::isnan(tYMax) ||
        std::isnan(tZMin) || std::isnan(tZMax)) {
        return true;
    }

    float tNearX = (std::min)(tXMin, tXMax);
    float tFarX = (std::max)(tXMin, tXMax);
    float tNearY = (std::min)(tYMin, tYMax);
    float tFarY = (std::max)(tYMin, tYMax);
    float tNearZ = (std::min)(tZMin, tZMax);
    float tFarZ = (std::max)(tZMin, tZMax);

    float tMin = (std::max)((std::max)(tNearX, tNearY), tNearZ);
    float tMax = (std::min)((std::min)(tFarX, tFarY), tFarZ);

    if (tMin > tMax) { return false; }
    // 始点側は判定無し
    if (tMin < 0.0f && tMax < 0.0f) { return false; }

    nearest.nearest = tMin;

    return true;
}
