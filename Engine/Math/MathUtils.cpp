#include "MathUtils.h"


#pragma region Vector2
const Vector2 Vector2::unitX{ 1.0f,0.0f };
const Vector2 Vector2::unitY{ 0.0f,1.0f };
const Vector2 Vector2::zero{ 0.0f,0.0f };
const Vector2 Vector2::one{ 1.0f,1.0f };
const Vector2 Vector2::right{ 1.0f,0.0f };
const Vector2 Vector2::left{ -1.0f,0.0f };
const Vector2 Vector2::up{ 0.0f,1.0f };
const Vector2 Vector2::down{ 0.0f,-1.0f };
const Vector2 Vector2::positiveInfinity{ Math::positiveInfinity, Math::positiveInfinity };
const Vector2 Vector2::negativeInfinity{ Math::negativeInfinity, Math::negativeInfinity };

Vector2::operator Vector3() const {
    return { x, y, 0.0f };
}

Vector2 Vector2::Slerp(float t, const Vector2& start, const Vector2& end) noexcept {
    float startLen = start.Length();
    float endLen = end.Length();
    Vector2 startNorm = start / startLen;
    Vector2 endNorm = end / endLen;

    float dot = Dot(startNorm, endNorm);
    if (std::abs(dot) > 0.999f) {
        return Lerp(t, start, end);
    }
    float theta = std::acos(dot);
    float invSinTheta = 1.0f / std::sin(theta);
    float t1 = std::sin((1.0f - t) * theta) * invSinTheta;
    float t2 = std::sin(t * theta) * invSinTheta;
    Vector2 direction = (t1 * startNorm + t2 * endNorm);
    float scale = startLen + t * (endLen - startLen);
    return direction * scale;
}

Vector2 Vector2::CatmullRomSpline(float t, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3) noexcept {
    return
        0.5f * ((-p0 + 3.0f * p1 - 3.0f * p2 + p3) * std::pow(t, 3.0f)
            + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * std::pow(t, 2.0f)
            + (-p0 + p2) * t
            + 2.0f * p1);
}

Vector2 Vector2::QuadraticBezierCurve(float t, const Vector2& p0, const Vector2& p1, const Vector2& p2) noexcept {
    float s = 1.0f - t;
    return
        s * s * p0 +
        2.0f * t * s * p1 +
        t * t * p2;
}

Vector2 Vector2::CubicBezierCurve(float t, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3) noexcept {
    float s = 1.0f - t;
    return
        (s * s * s * p0) +
        (3.0f * s * s * t * p1) +
        (3.0f * s * t * t * p2) +
        t * t * t * p3;
}
#pragma endregion
#pragma region Vector3
const Vector3 Vector3::zero{ 0.0f, 0.0f, 0.0f };
const Vector3 Vector3::unitX{ 1.0f, 0.0f, 0.0f };
const Vector3 Vector3::unitY{ 0.0f, 1.0f, 0.0f };
const Vector3 Vector3::unitZ{ 0.0f, 0.0f, 1.0f };
const Vector3 Vector3::one{ 1.0f, 1.0f, 1.0f };
const Vector3 Vector3::right{ 1.0f, 0.0f, 0.0f };
const Vector3 Vector3::left{ -1.0f, 0.0f, 0.0f };
const Vector3 Vector3::up{ 0.0f, 1.0f, 0.0f };
const Vector3 Vector3::down{ 0.0f, -1.0f, 0.0f };
const Vector3 Vector3::forward{ 0.0f, 0.0f, 1.0f };
const Vector3 Vector3::back{ 0.0f, 0.0f, -1.0f };
const Vector3 Vector3::positiveInfinity{ Math::positiveInfinity, Math::positiveInfinity, Math::positiveInfinity };
const Vector3 Vector3::negativeInfinity{ Math::negativeInfinity, Math::negativeInfinity, Math::negativeInfinity };

Vector3 Vector3::Slerp(float t, const Vector3& start, const Vector3& end) noexcept {
    float startLen = start.Length();
    float endLen = end.Length();
    Vector3 startNorm = start / startLen;
    Vector3 endNorm = end / endLen;

    float dot = Dot(startNorm, endNorm);
    if (std::abs(dot) > 0.999f) {
        return Lerp(t, start, end);
    }
    float theta = std::acos(dot);
    float invSinTheta = 1.0f / std::sin(theta);
    float t1 = std::sin((1.0f - t) * theta) * invSinTheta;
    float t2 = std::sin(t * theta) * invSinTheta;
    Vector3 direction = (t1 * startNorm + t2 * endNorm);
    float scale = startLen + t * (endLen - startLen);
    return direction * scale;
}

Vector3 Vector3::CatmullRomSpline(float t, const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3) noexcept {
    return
        0.5f * ((-p0 + 3.0f * p1 - 3.0f * p2 + p3) * std::pow(t, 3.0f)
            + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * std::pow(t, 2.0f)
            + (-p0 + p2) * t
            + 2.0f * p1);
}

Vector3 Vector3::QuadraticBezierCurve(float t, const Vector3& p0, const Vector3& p1, const Vector3& p2) noexcept {
    float s = 1.0f - t;
    return
        s * s * p0 +
        2.0f * t * s * p1 +
        t * t * p2;
}

Vector3 Vector3::CubicBezierCurve(float t, const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3) noexcept {
    float s = 1.0f - t;
    return
        (s * s * s * p0) +
        (3.0f * s * s * t * p1) +
        (3.0f * s * t * t * p2) +
        t * t * t * p3;
}
#pragma endregion
#pragma region Vector4
const Vector4 Vector4::zero = { 0.0f,0.0f,0.0f,0.0f };
const Vector4 Vector4::one = { 1.0f,1.0f,1.0f,1.0f };
#pragma endregion
#pragma region Quaternion
const Quaternion Quaternion::identity{ 0.0f,0.0f,0.0f,1.0f };
Quaternion Quaternion::MakeFromOrthonormal(const Vector3& x, const Vector3& y, const Vector3& z) noexcept {
    float trace = x.x + y.y + z.z;
    if (trace > 0.0f) {
        float s = std::sqrt(trace + 1.0f) * 0.5f;
        Quaternion result{};
        result.w = s;
        s = 0.25f / s;
        result.x = (y.z - z.y) * s;
        result.y = (z.x - x.z) * s;
        result.z = (x.y - y.x) * s;
        return result;
    }
    else if (x.x > y.y && x.x > z.z) {
        float s = std::sqrt(1.0f + x.x - y.y - z.z) * 0.5f;
        Quaternion result{};
        result.x = s;
        s = 0.25f / s;
        result.y = (x.y + y.x) * s;
        result.z = (z.x + x.z) * s;
        result.w = (y.z - z.y) * s;
        return result;
    }
    else if (y.y > z.z) {
        float s = std::sqrt(1.0f - x.x + y.y - z.z) * 0.5f;
        Quaternion result{};
        result.y = s;
        s = 0.25f / s;
        result.x = (x.y + y.x) * s;
        result.z = (y.z + z.y) * s;
        result.w = (z.x - x.z) * s;
        return result;
    }
    Quaternion result{};
    float s = std::sqrt(1.0f - x.x - y.y + z.z) * 0.5f;
    result.z = s;
    s = 0.25f / s;
    result.x = (z.x + x.z) * s;
    result.y = (y.z + z.y) * s;
    result.w = (x.y - y.x) * s;
    return result;
}
Quaternion Quaternion::Slerp(float t, const Quaternion& start, const Quaternion& end) noexcept {
    Quaternion s = start;
    float dot = Dot(start, end);
    if (dot > 0.9999f) {
        return Lerp(t, start, end);
    }
    // q1, q2が反対向きの場合
    if (dot < 0) {
        s.x = -s.x;
        s.y = -s.y;
        s.z = -s.z;
        s.w = -s.w;
        dot = -dot;
    }
    // 球面線形補間の計算
    float theta = std::acos(dot);
    return (std::sin((1.0f - t) * theta) * s + std::sin(t * theta) * end) * (1.0f / std::sin(theta));
}
#pragma endregion
#pragma region Matrix4x4
const Matrix4x4 Matrix4x4::identity{
    1.0f,0.0f,0.0f,0.0f,
    0.0f,1.0f,0.0f,0.0f,
    0.0f,0.0f,1.0f,0.0f,
    0.0f,0.0f,0.0f,1.0f };
float Matrix4x4::Determinant() const noexcept {
    float result = 0.0f;

    result += m[0][0] * m[1][1] * m[2][2] * m[3][3]; // +11,22,33,44
    result += m[0][0] * m[1][2] * m[2][3] * m[3][1]; // +11,23,34,42
    result += m[0][0] * m[1][3] * m[2][1] * m[3][2]; // +11,24,32,43

    result -= m[0][0] * m[1][3] * m[2][2] * m[3][1]; // -11,24,33,42
    result -= m[0][0] * m[1][2] * m[2][1] * m[3][3]; // -11,23,32,44
    result -= m[0][0] * m[1][1] * m[2][3] * m[3][2]; // -11,22,34,43

    result -= m[0][1] * m[1][0] * m[2][2] * m[3][3]; // -12,21,33,44
    result -= m[0][2] * m[1][0] * m[2][3] * m[3][1]; // -13,21,34,42
    result -= m[0][3] * m[1][0] * m[2][1] * m[3][2]; // -14,21,32,43

    result += m[0][3] * m[1][0] * m[2][2] * m[3][1]; // +14,21,33,42
    result += m[0][2] * m[1][0] * m[2][1] * m[3][3]; // +13,21,32,44
    result += m[0][1] * m[1][0] * m[2][3] * m[3][2]; // +12,21,34,43

    result += m[0][1] * m[1][2] * m[2][0] * m[3][3]; // +12,23,31,44
    result += m[0][2] * m[1][3] * m[2][0] * m[3][1]; // +13,24,31,42
    result += m[0][3] * m[1][1] * m[2][0] * m[3][2]; // +14,22,31,43

    result -= m[0][3] * m[1][2] * m[2][0] * m[3][1]; // -14,23,31,42
    result -= m[0][2] * m[1][1] * m[2][0] * m[3][3]; // -13,22,31,44
    result -= m[0][1] * m[1][3] * m[2][0] * m[3][2]; // -12,24,31,43

    result -= m[0][1] * m[1][2] * m[2][3] * m[3][0]; // -12,23,34,41
    result -= m[0][2] * m[1][3] * m[2][1] * m[3][0]; // -13,24,32,41
    result -= m[0][3] * m[1][1] * m[2][2] * m[3][0]; // -14,22,33,41

    result += m[0][3] * m[1][2] * m[2][1] * m[3][0]; // +14,23,32,41
    result += m[0][2] * m[1][1] * m[2][3] * m[3][0]; // +13,22,34,41
    result += m[0][1] * m[1][3] * m[2][2] * m[3][0]; // +12,24,33,41

    return result;
}
Matrix4x4 Matrix4x4::Adjugate() const noexcept {
    Matrix4x4 result;
    // 1行目
    result.m[0][0] = 0.0f;							// 11
    result.m[0][0] += m[1][1] * m[2][2] * m[3][3];	// +22,33,44
    result.m[0][0] += m[1][2] * m[2][3] * m[3][1];	// +23,34,42
    result.m[0][0] += m[1][3] * m[2][1] * m[3][2];	// +24,32,43
    result.m[0][0] -= m[1][3] * m[2][2] * m[3][1];	// -24,33,42
    result.m[0][0] -= m[1][2] * m[2][1] * m[3][3];	// -23,32,44
    result.m[0][0] -= m[1][1] * m[2][3] * m[3][2];	// -22,34,43

    result.m[0][1] = 0.0f;							// 12
    result.m[0][1] -= m[0][1] * m[2][2] * m[3][3];	// -12,33,44
    result.m[0][1] -= m[0][2] * m[2][3] * m[3][1];	// -13,34,42
    result.m[0][1] -= m[0][3] * m[2][1] * m[3][2];	// -14,32,43
    result.m[0][1] += m[0][3] * m[2][2] * m[3][1];	// +14,33,42
    result.m[0][1] += m[0][2] * m[2][1] * m[3][3];	// +13,32,44
    result.m[0][1] += m[0][1] * m[2][3] * m[3][2];	// +12,34,43

    result.m[0][2] = 0.0f;							// 13
    result.m[0][2] += m[0][1] * m[1][2] * m[3][3];	// +12,23,44
    result.m[0][2] += m[0][2] * m[1][3] * m[3][1];	// +13,24,42
    result.m[0][2] += m[0][3] * m[1][1] * m[3][2];	// +14,22,43
    result.m[0][2] -= m[0][3] * m[1][2] * m[3][1];	// -14,23,42
    result.m[0][2] -= m[0][2] * m[1][1] * m[3][3];	// -13,22,44
    result.m[0][2] -= m[0][1] * m[1][3] * m[3][2];	// -12,24,43

    result.m[0][3] = 0.0f;							// 14
    result.m[0][3] -= m[0][1] * m[1][2] * m[2][3];	// -12,23,34
    result.m[0][3] -= m[0][2] * m[1][3] * m[2][1];	// -13,24,32
    result.m[0][3] -= m[0][3] * m[1][1] * m[2][2];	// -14,22,33
    result.m[0][3] += m[0][3] * m[1][2] * m[2][1];	// +14,23,32
    result.m[0][3] += m[0][2] * m[1][1] * m[2][3];	// +13,22,34
    result.m[0][3] += m[0][1] * m[1][3] * m[2][2];	// +12,24,33


    // 2行目
    result.m[1][0] = 0.0f;							// 21
    result.m[1][0] -= m[1][0] * m[2][2] * m[3][3];	// -21,33,44
    result.m[1][0] -= m[1][2] * m[2][3] * m[3][0];	// -23,34,41
    result.m[1][0] -= m[1][3] * m[2][0] * m[3][2];	// -24,31,43
    result.m[1][0] += m[1][3] * m[2][2] * m[3][0];	// +24,33,41
    result.m[1][0] += m[1][2] * m[2][0] * m[3][3];	// +23,31,44
    result.m[1][0] += m[1][0] * m[2][3] * m[3][2];	// +21,34,43

    result.m[1][1] = 0.0f;							// 22
    result.m[1][1] += m[0][0] * m[2][2] * m[3][3];	// +11,33,44
    result.m[1][1] += m[0][2] * m[2][3] * m[3][0];	// +13,34,41
    result.m[1][1] += m[0][3] * m[2][0] * m[3][2];	// +14,31,43
    result.m[1][1] -= m[0][3] * m[2][2] * m[3][0];	// -14,33,41
    result.m[1][1] -= m[0][2] * m[2][0] * m[3][3];	// -13,31,44
    result.m[1][1] -= m[0][0] * m[2][3] * m[3][2];	// -11,34,43

    result.m[1][2] = 0.0f;							// 23
    result.m[1][2] -= m[0][0] * m[1][2] * m[3][3];	// -11,23,44
    result.m[1][2] -= m[0][2] * m[1][3] * m[3][0];	// -13,24,41
    result.m[1][2] -= m[0][3] * m[1][0] * m[3][2];	// -14,21,43
    result.m[1][2] += m[0][3] * m[1][2] * m[3][0];	// +14,23,41
    result.m[1][2] += m[0][2] * m[1][0] * m[3][3];	// +13,21,44
    result.m[1][2] += m[0][0] * m[1][3] * m[3][2];	// +11,24,43

    result.m[1][3] = 0.0f;							// 24
    result.m[1][3] += m[0][0] * m[1][2] * m[2][3];	// +11,23,34
    result.m[1][3] += m[0][2] * m[1][3] * m[2][0];	// +13,24,31
    result.m[1][3] += m[0][3] * m[1][0] * m[2][2];	// +14,21,33
    result.m[1][3] -= m[0][3] * m[1][2] * m[2][0];	// -14,23,31
    result.m[1][3] -= m[0][2] * m[1][0] * m[2][3];	// -13,21,34
    result.m[1][3] -= m[0][0] * m[1][3] * m[2][2];	// -11,24,33


    // 3行目
    result.m[2][0] = 0.0f;							// 31
    result.m[2][0] += m[1][0] * m[2][1] * m[3][3];	// +21,32,44
    result.m[2][0] += m[1][1] * m[2][3] * m[3][0];	// +22,34,41
    result.m[2][0] += m[1][3] * m[2][0] * m[3][1];	// +24,31,42
    result.m[2][0] -= m[1][3] * m[2][1] * m[3][0];	// -24,32,41
    result.m[2][0] -= m[1][1] * m[2][0] * m[3][3];	// -22,31,44
    result.m[2][0] -= m[1][0] * m[2][3] * m[3][1];	// -21,34,42

    result.m[2][1] = 0.0f;							// 32
    result.m[2][1] -= m[0][0] * m[2][1] * m[3][3];	// -11,32,44
    result.m[2][1] -= m[0][1] * m[2][3] * m[3][0];	// -12,34,41
    result.m[2][1] -= m[0][3] * m[2][0] * m[3][1];	// -14,31,42
    result.m[2][1] += m[0][3] * m[2][1] * m[3][0];	// +14,32,41
    result.m[2][1] += m[0][1] * m[2][0] * m[3][3];	// +12,31,44
    result.m[2][1] += m[0][0] * m[2][3] * m[3][1];	// +11,34,42

    result.m[2][2] = 0.0f;							// 33
    result.m[2][2] += m[0][0] * m[1][1] * m[3][3];	// +11,22,44
    result.m[2][2] += m[0][1] * m[1][3] * m[3][0];	// +12,24,41
    result.m[2][2] += m[0][3] * m[1][0] * m[3][1];	// +14,21,42
    result.m[2][2] -= m[0][3] * m[1][1] * m[3][0];	// -14,22,41
    result.m[2][2] -= m[0][1] * m[1][0] * m[3][3];	// -12,21,44
    result.m[2][2] -= m[0][0] * m[1][3] * m[3][1];	// -11,24,42

    result.m[2][3] = 0.0f;							// 34
    result.m[2][3] -= m[0][0] * m[1][1] * m[2][3];	// -11,22,34
    result.m[2][3] -= m[0][1] * m[1][3] * m[2][0];	// -12,24,31
    result.m[2][3] -= m[0][3] * m[1][0] * m[2][1];	// -14,21,32
    result.m[2][3] += m[0][3] * m[1][1] * m[2][0];	// +14,22,31
    result.m[2][3] += m[0][1] * m[1][0] * m[2][3];	// +12,21,34
    result.m[2][3] += m[0][0] * m[1][3] * m[2][1];	// +11,24,32


    // 4行目
    result.m[3][0] = 0.0f;							// 41
    result.m[3][0] -= m[1][0] * m[2][1] * m[3][2];	// -21,32,43
    result.m[3][0] -= m[1][1] * m[2][2] * m[3][0];	// -22,33,41
    result.m[3][0] -= m[1][2] * m[2][0] * m[3][1];	// -23,31,42
    result.m[3][0] += m[1][2] * m[2][1] * m[3][0];	// +23,32,41
    result.m[3][0] += m[1][1] * m[2][0] * m[3][2];	// +22,31,43
    result.m[3][0] += m[1][0] * m[2][2] * m[3][1];	// +21,33,42

    result.m[3][1] = 0.0f;							// 42
    result.m[3][1] += m[0][0] * m[2][1] * m[3][2];	// +11,32,43
    result.m[3][1] += m[0][1] * m[2][2] * m[3][0];	// +12,33,41
    result.m[3][1] += m[0][2] * m[2][0] * m[3][1];	// +13,31,42
    result.m[3][1] -= m[0][2] * m[2][1] * m[3][0];	// -13,32,41
    result.m[3][1] -= m[0][1] * m[2][0] * m[3][2];	// -12,31,43
    result.m[3][1] -= m[0][0] * m[2][2] * m[3][1];	// -11,33,42

    result.m[3][2] = 0.0f;							// 43
    result.m[3][2] -= m[0][0] * m[1][1] * m[3][2];	// -11,22,43
    result.m[3][2] -= m[0][1] * m[1][2] * m[3][0];	// -12,23,41
    result.m[3][2] -= m[0][2] * m[1][0] * m[3][1];	// -13,21,42
    result.m[3][2] += m[0][2] * m[1][1] * m[3][0];	// +13,22,41
    result.m[3][2] += m[0][1] * m[1][0] * m[3][2];	// +12,21,43
    result.m[3][2] += m[0][0] * m[1][2] * m[3][1];	// +11,23,42

    result.m[3][3] = 0.0f;							// 44
    result.m[3][3] += m[0][0] * m[1][1] * m[2][2];	// +11,22,33
    result.m[3][3] += m[0][1] * m[1][2] * m[2][0];	// +12,23,31
    result.m[3][3] += m[0][2] * m[1][0] * m[2][1];	// +13,21,32
    result.m[3][3] -= m[0][2] * m[1][1] * m[2][0];	// -13,22,31
    result.m[3][3] -= m[0][1] * m[1][0] * m[2][2];	// -12,21,33
    result.m[3][3] -= m[0][0] * m[1][2] * m[2][1];	// -11,23,32

    return result;
}
#pragma endregion

