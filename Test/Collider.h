#pragma once
#include <iostream>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Engine/Graphics/RenderManager.h"
#include "Externals/nlohmann/json.hpp"
#include "Engine/File/JsonConverter.h"

enum class CollisionCategory : uint32_t {
	NONE = 0,
	PLAYER = 1 << 0,
	FLASHLIGHT = 1 << 1,
	LIGHT = 1 << 2,
	ENEMY = 1 << 3,
	GIMMICKTRIGGER = 1 << 4,
	GIMMICKMOVER = 1 << 5,
	GIMMICKPOINTLIGHT = 1 << 6,

	ALL = ~0u
};

inline CollisionCategory operator|(CollisionCategory a, CollisionCategory b) {
	return static_cast<CollisionCategory>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

struct SphereCollider;
struct BoxCollider;
struct OBBCollider;
struct ConeCollider;
struct CapsuleCollider;
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
	Collider(CollisionCategory category, CollisionCategory mask, Vector3 pos)
		: categoryBits(category),
		maskBits(static_cast<uint32_t>(mask)),
		center(pos) {
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
	virtual bool CheckCollision(CapsuleCollider& other) = 0;

	virtual void OnCollision(Collider& other) {
		collidedWith.push_back(&other);
	}

	virtual void DrawDebug(const Vector4& color) const = 0;

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

	SphereCollider(CollisionCategory category, CollisionCategory mask, Vector3 pos, float r)
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
	bool CheckCollision(CapsuleCollider& other) override;

	void DrawDebug(const Vector4& color) const override;
};

struct BoxCollider : public Collider {
	Vector3 size;

	BoxCollider(CollisionCategory category, CollisionCategory mask, Vector3 pos, Vector3 s)
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
		return GetWorldCenter() - (GetWorldSize() * 0.5f);
	}

	Vector3 GetWorldMax() const {
		return GetWorldCenter() + (GetWorldSize() * 0.5f);
	}

	bool CheckCollision(Collider& other) override;
	bool CheckCollision(SphereCollider& other) override;
	bool CheckCollision(BoxCollider& other) override;
	bool CheckCollision(OBBCollider& other) override;
	bool CheckCollision(ConeCollider& other) override;
	bool CheckCollision(CapsuleCollider& other) override;

	void DrawDebug(const Vector4& color) const override;
};

struct OBBCollider : public Collider {
	Vector3 size;
	Quaternion quaternion;



	OBBCollider(CollisionCategory category, CollisionCategory mask, Vector3 pos, Vector3 s, Quaternion rot)
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
	bool CheckCollision(CapsuleCollider& other) override;

	void DrawDebug(const Vector4& color) const override;
};

struct ConeCollider : public Collider {
	float radius;
	float height;
	Quaternion quaternion;

	ConeCollider(CollisionCategory category, CollisionCategory mask, Vector3 pos, float r, float h, Quaternion rot)
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
	bool CheckCollision(CapsuleCollider& other) override;

	void DrawDebug(const Vector4& color) const override;
};

struct CapsuleCollider : public Collider {
	float radius;
	float height;
	Quaternion quaternion;

	CapsuleCollider(CollisionCategory category, CollisionCategory mask, Vector3 pos, float r, float h, Quaternion rot)
		: Collider(category, mask, pos), radius(r), height(h), quaternion(rot) {
	}

	Quaternion GetWorldOrientation() const {
		if (parent) {
			Quaternion parentWorldRot = parent->worldMatrix.GetRotate();
			return parentWorldRot * quaternion;
		}
		return quaternion;
	}

	float GetWorldRadius() const {
		if (parent) {
			Vector3 s = parent->worldMatrix.GetScale();
			float maxScaleXZ = std::max(std::abs(s.x), std::abs(s.z));
			return radius * maxScaleXZ;
		}
		return radius;
	}

	float GetWorldHeight() const {
		if (parent) {
			Vector3 s = parent->worldMatrix.GetScale();
			return height * std::abs(s.y);
		}
		return height;
	}

	void GetWorldSegment(Vector3& outStart, Vector3& outEnd) const {
		Vector3 centerPos = GetWorldCenter();
		Quaternion rot = GetWorldOrientation();
		float h = GetWorldHeight();

		// Y軸方向に伸びると仮定
		// 中心から上下に h/2 ずつ
		Vector3 offset = rot * Vector3(0, h * 0.5f, 0);
		outStart = centerPos - offset;
		outEnd = centerPos + offset;
	}

	bool CheckCollision(Collider& other) override;
	bool CheckCollision(SphereCollider& other) override;
	bool CheckCollision(BoxCollider& other) override;
	bool CheckCollision(OBBCollider& other) override;
	bool CheckCollision(ConeCollider& other) override;
	bool CheckCollision(CapsuleCollider& other) override;

	void DrawDebug(const Vector4& color) const override;
};

#pragma region 当たり判定ヘルパー

// 線分(p1-p2)と点(point)の最短距離の2乗を返す
inline float DistSqPointSegment(const Vector3& p1, const Vector3& p2, const Vector3& point) {
	Vector3 d = p2 - p1;
	float lenSq = d.LengthSquare();
	if (lenSq == 0.0f) return (point - p1).LengthSquare(); // 線分が点の場合

	// 投影係数 t = Dot(ap, ab) / Dot(ab, ab)
	float t = Vector3::Dot(point - p1, d) / lenSq;
	t = std::clamp(t, 0.0f, 1.0f);

	Vector3 closest = p1 + d * t;
	return (point - closest).LengthSquare();
}

// 線分(p1-q1) と 線分(p2-q2) の最短距離の2乗を返す
inline float DistSqSegmentSegment(const Vector3& p1, const Vector3& q1, const Vector3& p2, const Vector3& q2) {
	Vector3 d1 = q1 - p1;
	Vector3 d2 = q2 - p2;
	Vector3 r = p1 - p2;
	float a = d1.LengthSquare();
	float e = d2.LengthSquare();
	float f = Vector3::Dot(d2, r);

	// 両方の線分が点の場合
	if (a <= 1e-6f && e <= 1e-6f) return r.LengthSquare();

	// 第1線分が点
	if (a <= 1e-6f) return DistSqPointSegment(p2, q2, p1);

	// 第2線分が点
	if (e <= 1e-6f) return DistSqPointSegment(p1, q1, p2);

	float c = Vector3::Dot(d1, r);
	float b = Vector3::Dot(d1, d2);
	float denom = a * e - b * b;

	float s, t;

	// 平行でない場合
	if (denom != 0.0f) {
		s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
	}
	else {
		s = 0.0f; // 平行の場合、任意のs (ここでは0)
	}

	// sに基づいてtを計算
	float tNom = b * s + f;
	if (tNom < 0.0f) {
		t = 0.0f;
		s = std::clamp(-c / a, 0.0f, 1.0f);
	}
	else if (tNom > e) {
		t = 1.0f;
		s = std::clamp((b - c) / a, 0.0f, 1.0f);
	}
	else {
		t = tNom / e;
	}

	Vector3 closestPoint1 = p1 + d1 * s;
	Vector3 closestPoint2 = p2 + d2 * t;
	return (closestPoint1 - closestPoint2).LengthSquare();
}

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

	float coneH = cone.height;
	float coneR = cone.radius;

	float axisY = localPos.y;
	float axisR = sqrt(localPos.x * localPos.x + localPos.z * localPos.z);

	float edgeX = coneR;
	float edgeY = -coneH;
	float edgeLenSq = edgeX * edgeX + edgeY * edgeY;
	float pX = axisR;
	float pY = axisY - coneH;

	float t = 0.0f;
	if (edgeLenSq > 1e-6f) {
		t = (pX * edgeX + pY * edgeY) / edgeLenSq;
	}
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
inline bool SphereCollider::CheckCollision(CapsuleCollider& other) {
	Vector3 p1, p2;
	other.GetWorldSegment(p1, p2);
	float distSq = DistSqPointSegment(p1, p2, GetWorldCenter());
	float rSum = GetWorldRadius() + other.GetWorldRadius();
	return distSq <= rSum * rSum;
}

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
	OBBCollider temp(CollisionCategory::NONE, CollisionCategory::NONE, GetWorldCenter(), GetWorldSize(), id);
	return CheckOBBvsOBB(temp, other);
}
inline bool BoxCollider::CheckCollision(ConeCollider& other) { return other.CheckCollision(*this); }

inline bool BoxCollider::CheckCollision(CapsuleCollider& other) {
	Vector3 p1, p2;
	other.GetWorldSegment(p1, p2);
	float capsuleRadius = other.GetWorldRadius();
	float capsuleRadiusSq = capsuleRadius * capsuleRadius;

	Vector3 boxMin = GetWorldMin();
	Vector3 boxMax = GetWorldMax();

	Vector3 d = p2 - p1;
	float segLenSq = d.LengthSquare();

	float t = 0.0f;
	if (segLenSq > 1e-6f) {
		// Boxの中心
		Vector3 boxCenter = (boxMin + boxMax) * 0.5f;
		t = Vector3::Dot(boxCenter - p1, d) / segLenSq;
		t = std::clamp(t, 0.0f, 1.0f);
	}

	Vector3 closestOnSegment;
	Vector3 closestOnBox;

	for (int i = 0; i < 4; ++i) {
		closestOnSegment = p1 + d * t;

		closestOnBox.x = std::clamp(closestOnSegment.x, boxMin.x, boxMax.x);
		closestOnBox.y = std::clamp(closestOnSegment.y, boxMin.y, boxMax.y);
		closestOnBox.z = std::clamp(closestOnSegment.z, boxMin.z, boxMax.z);

		if (segLenSq <= 1e-6f) {
			break;
		}

		t = Vector3::Dot(closestOnBox - p1, d) / segLenSq;
		t = std::clamp(t, 0.0f, 1.0f);
	}

	closestOnSegment = p1 + d * t;
	closestOnBox.x = std::clamp(closestOnSegment.x, boxMin.x, boxMax.x);
	closestOnBox.y = std::clamp(closestOnSegment.y, boxMin.y, boxMax.y);
	closestOnBox.z = std::clamp(closestOnSegment.z, boxMin.z, boxMax.z);

	float distSq = (closestOnSegment - closestOnBox).LengthSquare();

	return distSq <= capsuleRadiusSq;
}

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

inline bool OBBCollider::CheckCollision(CapsuleCollider& other) {
	Vector3 p1, p2;
	other.GetWorldSegment(p1, p2);
	Vector3 obbCenter = GetWorldCenter();
	// 線分上でOBB中心に近い点を探す（簡易）
	// ※より厳密にはOBB空間に線分を持っていく必要があるが、簡易実装としてSphere近似
	Vector3 d = p2 - p1;
	float t = Vector3::Dot(obbCenter - p1, d) / d.LengthSquare();
	t = std::clamp(t, 0.0f, 1.0f);
	Vector3 closestOnSegment = p1 + d * t;

	SphereCollider tempSphere(CollisionCategory::NONE, CollisionCategory::NONE, closestOnSegment, other.GetWorldRadius());
	return CheckCollision(tempSphere);
}

inline bool ConeCollider::CheckCollision(Collider& other) { return other.CheckCollision(*this); }

inline bool ConeCollider::CheckCollision(SphereCollider& other) {
	return CheckConeVsSphere(*this, other);
}

inline bool ConeCollider::CheckCollision(BoxCollider& other) {
	float boxRadius = other.GetWorldSize().Length() * 0.5f;
	float coneBoundingRadius = std::max(height, radius * 1.5f);

	Vector3 conePos = GetWorldCenter();
	Vector3 boxPos = other.GetWorldCenter();

	if ((conePos - boxPos).LengthSquare() > std::pow(boxRadius + coneBoundingRadius, 2)) {
		return false;
	}

	Quaternion coneRot = GetWorldOrientation();
	Quaternion invConeRot = coneRot.Conjugate();

	auto IsPointInCone = [&](const Vector3& point) -> bool {
		Vector3 localPos = invConeRot * (point - conePos);

		if (localPos.y < 0.0f || localPos.y > height) {
			return false;
		}

		float rAtY = radius * (1.0f - (localPos.y / height));

		return (localPos.x * localPos.x + localPos.z * localPos.z) <= (rAtY * rAtY);
		};

	Vector3 boxMin = other.GetWorldMin();
	Vector3 boxMax = other.GetWorldMax();

	Vector3 corners[8] = {
		{ boxMin.x, boxMin.y, boxMin.z }, { boxMax.x, boxMin.y, boxMin.z },
		{ boxMin.x, boxMax.y, boxMin.z }, { boxMax.x, boxMax.y, boxMin.z },
		{ boxMin.x, boxMin.y, boxMax.z }, { boxMax.x, boxMin.y, boxMax.z },
		{ boxMin.x, boxMax.y, boxMax.z }, { boxMax.x, boxMax.y, boxMax.z }
	};

	for (const auto& p : corners) {
		if (IsPointInCone(p)) {
			return true;
		}
	}

	Vector3 axisY = coneRot * Vector3(0, 1, 0);
	const int checkCount = 3;

	for (int i = 0; i <= checkCount; ++i) {
		float t = static_cast<float>(i) / checkCount;
		Vector3 pointOnAxis = conePos + axisY * (height * t);

		float cx = std::clamp(pointOnAxis.x, boxMin.x, boxMax.x);
		float cy = std::clamp(pointOnAxis.y, boxMin.y, boxMax.y);
		float cz = std::clamp(pointOnAxis.z, boxMin.z, boxMax.z);
		Vector3 closestOnBox = { cx, cy, cz };

		if (IsPointInCone(closestOnBox)) {
			return true;
		}
	}

	return false;
}

inline bool ConeCollider::CheckCollision(OBBCollider& other) {
	float obbRadius = other.GetWorldSize().Length() * 0.5f;
	float coneBoundingRadius = std::max(height, radius * 1.5f);

	Vector3 conePos = GetWorldCenter();
	Vector3 obbPos = other.GetWorldCenter();

	if ((conePos - obbPos).LengthSquare() > std::pow(obbRadius + coneBoundingRadius, 2)) {
		return false;
	}

	Quaternion coneRot = GetWorldOrientation();
	Quaternion invConeRot = coneRot.Conjugate();

	Quaternion obbRot = other.GetWorldOrientation();
	Quaternion invObbRot = obbRot.Conjugate();
	Vector3 obbExtents = other.GetWorldSize() * 0.5f;

	auto IsPointInCone = [&](const Vector3& worldPoint) -> bool {
		Vector3 localPos = invConeRot * (worldPoint - conePos);

		if (localPos.y < 0.0f || localPos.y > height) return false;

		float rAtY = radius * (1.0f - (localPos.y / height));
		return (localPos.x * localPos.x + localPos.z * localPos.z) <= (rAtY * rAtY);
		};

	Vector3 cornersLocal[8] = {
		{-obbExtents.x, -obbExtents.y, -obbExtents.z}, { obbExtents.x, -obbExtents.y, -obbExtents.z},
		{-obbExtents.x,  obbExtents.y, -obbExtents.z}, { obbExtents.x,  obbExtents.y, -obbExtents.z},
		{-obbExtents.x, -obbExtents.y,  obbExtents.z}, { obbExtents.x, -obbExtents.y,  obbExtents.z},
		{-obbExtents.x,  obbExtents.y,  obbExtents.z}, { obbExtents.x,  obbExtents.y,  obbExtents.z}
	};

	for (const auto& offset : cornersLocal) {
		Vector3 worldCorner = obbPos + (obbRot * offset);

		if (IsPointInCone(worldCorner)) {
			return true;
		}
	}

	Vector3 coneAxisY = coneRot * Vector3(0, 1, 0);
	const int checkCount = 3;

	for (int i = 0; i <= checkCount; ++i) {
		float t = static_cast<float>(i) / checkCount;
		Vector3 pointOnAxis = conePos + coneAxisY * (height * t);

		Vector3 localPointInOBB = invObbRot * (pointOnAxis - obbPos);

		Vector3 closestLocalInOBB = {
			std::clamp(localPointInOBB.x, -obbExtents.x, obbExtents.x),
			std::clamp(localPointInOBB.y, -obbExtents.y, obbExtents.y),
			std::clamp(localPointInOBB.z, -obbExtents.z, obbExtents.z)
		};

		Vector3 closestPointWorld = obbPos + (obbRot * closestLocalInOBB);

		if (IsPointInCone(closestPointWorld)) {
			return true;
		}
	}

	return false;
}

inline bool ConeCollider::CheckCollision(ConeCollider& other) {
	//このあたり判定はしないで
	other;
	return false;
}

inline bool ConeCollider::CheckCollision(CapsuleCollider& other) {
	float capH = other.GetWorldHeight();
	float capR = other.GetWorldRadius();
	float capBoundingR = (capH * 0.5f) + capR;

	float coneBoundingR = std::max(height, radius * 1.5f);

	Vector3 conePos = GetWorldCenter();
	Vector3 capPos = other.GetWorldCenter();

	if ((conePos - capPos).LengthSquare() > std::pow(capBoundingR + coneBoundingR, 2)) {
		return false;
	}

	Vector3 capStart, capEnd;
	other.GetWorldSegment(capStart, capEnd);

	Quaternion coneRot = GetWorldOrientation();
	Vector3 coneAxisY = coneRot * Vector3(0, 1, 0); // ローカルY軸
	Vector3 coneTip = conePos + coneAxisY * height;

	auto CheckSphere = [&](const Vector3& point) -> bool {
		SphereCollider tempSphere(CollisionCategory::NONE, CollisionCategory::NONE, point, capR);
		return CheckConeVsSphere(*this, tempSphere);
		};

	if (CheckSphere(capStart)) return true;
	if (CheckSphere(capEnd)) return true;

	Vector3 p1 = conePos;
	Vector3 p2 = coneTip;
	Vector3 q1 = capStart;
	Vector3 q2 = capEnd;

	Vector3 d1 = p2 - p1;
	Vector3 d2 = q2 - q1;
	Vector3 r = p1 - q1;
	float a = d1.LengthSquare();
	float e = d2.LengthSquare();
	float f = Vector3::Dot(d2, r);

	float b = Vector3::Dot(d1, d2);
	float c = Vector3::Dot(d1, r);
	float denom = a * e - b * b;

	float t = 0.0f;

	if (denom != 0.0f) {
		t = (b * c - a * f) / denom;
	}
	else {
		t = 0.5f;
	}

	t = std::clamp(t, 0.0f, 1.0f);

	Vector3 closestOnCapsule = q1 + d2 * t;

	if (t > 0.01f && t < 0.99f) {
		if (CheckSphere(closestOnCapsule)) return true;
	}

	return false;
}

inline bool CapsuleCollider::CheckCollision(Collider& other) { return other.CheckCollision(*this); }
inline bool CapsuleCollider::CheckCollision(SphereCollider& other) {
	return other.CheckCollision(*this);
}
inline bool CapsuleCollider::CheckCollision(BoxCollider& other) {
	return other.CheckCollision(*this);
}
inline bool CapsuleCollider::CheckCollision(OBBCollider& other) {
	return other.CheckCollision(*this);
}
inline bool CapsuleCollider::CheckCollision(ConeCollider& other) {
	return other.CheckCollision(*this);
}

inline bool CapsuleCollider::CheckCollision(CapsuleCollider& other) {
	Vector3 p1, p2, q1, q2;
	GetWorldSegment(p1, p2);
	other.GetWorldSegment(q1, q2);

	float distSq = DistSqSegmentSegment(p1, p2, q1, q2);
	float rSum = GetWorldRadius() + other.GetWorldRadius();
	return distSq <= rSum * rSum;
}

#pragma region DebugDraw

inline void SphereCollider::DrawDebug(const Vector4& color) const {
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
	lineDrawer.DrawSphere(GetWorldCenter(), GetWorldRadius(), color);
}

inline void BoxCollider::DrawDebug(const Vector4& color) const {
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
	lineDrawer.DrawBox(GetWorldCenter(), GetWorldSize(), color);
}
inline void OBBCollider::DrawDebug(const Vector4& color) const {
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
	lineDrawer.ObbDraw(
		GetWorldCenter(),
		GetWorldSize(),
		GetWorldOrientation(),
		color
	);
}

inline void ConeCollider::DrawDebug(const Vector4& color) const {
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
	lineDrawer.DrawCone(
		GetWorldCenter(),
		radius,
		height,
		GetWorldOrientation(),
		color
	);
}

inline void CapsuleCollider::DrawDebug(const Vector4& color) const {
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();

	Vector3 start, end;
	GetWorldSegment(start, end);
	float r = GetWorldRadius();

	lineDrawer.DrawSphere(start, r, color);
	lineDrawer.DrawSphere(end, r, color);

	Quaternion rot = GetWorldOrientation();
	Vector3 right = rot * Vector3(1, 0, 0) * r;
	Vector3 fwd = rot * Vector3(0, 0, 1) * r;

	lineDrawer.AddLine(start + right, end + right, color);
	lineDrawer.AddLine(start - right, end - right, color);
	lineDrawer.AddLine(start + fwd, end + fwd, color);
	lineDrawer.AddLine(start - fwd, end - fwd, color);
}
#pragma endregion
