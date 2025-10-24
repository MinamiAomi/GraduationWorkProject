#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

#include <DirectXMath.h>

namespace Math {
    inline constexpr float Pi = 3.141592653589793f;
    inline constexpr float TwoPi = Pi * 2.0f;
    inline constexpr float HalfPi = Pi * 0.5f;
    inline constexpr float ToRadian = Pi / 180.0f;
    inline constexpr float ToDegree = 180.0f / Pi;
    inline constexpr float positiveInfinity = (std::numeric_limits<float>::max)();
    inline constexpr float negativeInfinity = (std::numeric_limits<float>::min)();

    inline uint32_t Digit(int32_t num) {
        uint32_t digit = 1;
        int32_t tmp = abs(num);
        while (!(tmp < 10)) {
            tmp = tmp / 10;
            digit++;
        }
        return digit;
    }

    inline constexpr float Lerp(float t, float start, float end) {
        return start + t * (end - start);
    }

}

#pragma region 前方宣言
class Vector2;
class Vector3;
class Vector4;
class Quaternion;
class Matrix4x4;
#pragma endregion

class Vector2 {
public:
#pragma region 定数
    static const Vector2 zero;
    static const Vector2 unitX;
    static const Vector2 unitY;
    static const Vector2 one;
    static const Vector2 right;
    static const Vector2 left;
    static const Vector2 up;
    static const Vector2 down;
    static const Vector2 positiveInfinity;
    static const Vector2 negativeInfinity;

#pragma endregion
#pragma region コンストラクタ
    inline constexpr Vector2() noexcept : x(0.0f), y(0.0f) {}
    inline constexpr Vector2(float x, float y) noexcept : x(x), y(y) {}
    inline constexpr explicit Vector2(float xy) noexcept : x(xy), y(xy) {}
#pragma endregion
#pragma region 演算子のオーバーロード
    inline constexpr const float& operator[](size_t index) const noexcept {
        return *(&x + index);
    }
    inline constexpr float& operator[](size_t index) noexcept {
        return const_cast<float&>(static_cast<const Vector2&>(*this)[index]);
    }

    explicit operator Vector3() const;

    friend inline constexpr Vector2 operator+(const Vector2& rhs) noexcept {
        return rhs;
    }
    friend inline constexpr Vector2 operator-(const Vector2& rhs) noexcept {
        return { -rhs.x, -rhs.y };
    }
    friend inline constexpr Vector2 operator+(const Vector2& lhs, const Vector2& rhs) noexcept {
        return { lhs.x + rhs.x, lhs.y + rhs.y };
    }
    friend inline constexpr Vector2 operator-(const Vector2& lhs, const Vector2& rhs) noexcept {
        return { lhs.x - rhs.x, lhs.y - rhs.y };
    }
    friend inline constexpr Vector2 operator*(const Vector2& lhs, float rhs) noexcept {
        return { lhs.x * rhs, lhs.y * rhs };
    }
    friend inline constexpr Vector2 operator*(float lhs, const Vector2& rhs) noexcept {
        return { lhs * rhs.x, lhs * rhs.y };
    }
    friend inline constexpr Vector2 operator/(const Vector2& lhs, float rhs) noexcept {
        return lhs * (1.0f / rhs);
    }
    friend inline constexpr float Dot(const Vector2& lhs, const Vector2& rhs) noexcept {
        return Vector2::Dot(lhs, rhs);
    }
    friend inline constexpr float Cross(const Vector2& lhs, const Vector2& rhs) noexcept {
        return Vector2::Cross(lhs, rhs);
    }
    friend inline constexpr Vector2& operator+=(Vector2& lhs, const Vector2& rhs) noexcept {
        lhs = lhs + rhs;
        return lhs;
    }
    friend inline constexpr Vector2& operator-=(Vector2& lhs, const Vector2& rhs) noexcept {
        lhs = lhs - rhs;
        return lhs;
    }
    friend inline constexpr Vector2& operator*=(Vector2& lhs, float rhs) noexcept {
        lhs = lhs * rhs;
        return lhs;
    }
    friend inline constexpr Vector2& operator/=(Vector2& lhs, float rhs) noexcept {
        lhs = lhs / rhs;
        return lhs;
    }
    friend inline constexpr bool operator==(const Vector2& lhs, const Vector2& rhs) noexcept {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
    friend inline constexpr bool operator!=(const Vector2& lhs, const Vector2& rhs) noexcept {
        return !(lhs == rhs);
    }
#pragma endregion
#pragma region メンバ関数
    inline constexpr float LengthSquare() const noexcept {
        return x * x + y * y;
    }
    inline float Length() const noexcept {
        return std::sqrt(LengthSquare());
    }
    inline Vector2 Normalized() const noexcept {
        return *this / Length();
    }
    inline Vector2& Normalize() noexcept {
        return (*this = Normalized());
    }
#pragma endregion
#pragma region 静的関数
    static inline float Angle(const Vector2& from, const Vector2& to) noexcept {
        return std::acos(Dot(from.Normalized(), to.Normalized()));
    }
    static inline float SignedAngle(const Vector2& from, const Vector2& to) noexcept {
        return Cross(from, to) >= 0.0f ? Angle(from, to) : -Angle(from, to);
    }
    static inline float Distance(const Vector2& start, const Vector2& end) noexcept {
        return (end - start).Length();
    }
    static inline constexpr float Dot(const Vector2& lhs, const Vector2& rhs) noexcept {
        return lhs.x * rhs.x + lhs.y * rhs.y;
    }
    static inline constexpr float Cross(const Vector2& lhs, const Vector2& rhs) noexcept {
        return lhs.x * rhs.y - lhs.y * rhs.x;
    }
    static inline constexpr Vector2 Triple(const Vector2& a, const Vector2& b, const Vector2& c) {
        return b * Dot(a, c) - a * Dot(b, c);
    }
    static inline constexpr Vector2 Perpendicular(const Vector2& direction) noexcept {
        return { -direction.y, direction.x };
    }
    static inline constexpr Vector2 TrpleProduct(const Vector2& a, const Vector2& b, const Vector2& c) noexcept {
        return b * Dot(a, c) - a * Dot(b, c);
    }
    static inline constexpr Vector2 Scale(const Vector2& lhs, const Vector2& rhs) noexcept {
        return { lhs.x * rhs.x, lhs.y * rhs.y };
    }
    static inline constexpr Vector2 Project(const Vector2& base, const Vector2& direction) noexcept {
        return Dot(base, direction) * direction;
    }
    static inline constexpr Vector2 Reflecte(const Vector2& direction, const Vector2& normal) noexcept {
        return Dot(normal, -direction) * 2.0f * normal + direction;
    }
    static inline constexpr Vector2 Min(const Vector2& lhs, const Vector2& rhs) noexcept {
        return { std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y) };
    }
    static inline constexpr Vector2 Max(const Vector2& lhs, const Vector2& rhs) noexcept {
        return { std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y) };
    }
    static inline constexpr Vector2 Clamp(const Vector2& value, const Vector2& min, const Vector2& max) noexcept {
        return { std::clamp(value.x, min.x, max.x), std::clamp(value.y, min.y, max.y) };
    }
    static inline constexpr Vector2 Lerp(float t, const Vector2& start, const Vector2& end) noexcept {
        return start + t * (end - start);
    }
    static Vector2 Slerp(float t, const Vector2& start, const Vector2& end) noexcept;
    static Vector2 CatmullRomSpline(float t, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3) noexcept;
    static Vector2 QuadraticBezierCurve(float t, const Vector2& p0, const Vector2& p1, const Vector2& p2) noexcept;
    static Vector2 CubicBezierCurve(float t, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3) noexcept;
#pragma endregion
#pragma region メンバ変数
    float x, y;
#pragma endregion
};

class Vector3 {
public:
#pragma region 定数
    static const Vector3 zero;
    static const Vector3 unitX;
    static const Vector3 unitY;
    static const Vector3 unitZ;
    static const Vector3 one;
    static const Vector3 right;
    static const Vector3 left;
    static const Vector3 up;
    static const Vector3 down;
    static const Vector3 forward;
    static const Vector3 back;
    static const Vector3 positiveInfinity;
    static const Vector3 negativeInfinity;
#pragma endregion
#pragma region コンストラクタ
    inline constexpr Vector3() noexcept : x(0.0f), y(0.0f), z(0.0f) {}
    inline constexpr Vector3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
    inline constexpr Vector3(const Vector2& xy, float z) noexcept : x(xy.x), y(xy.y), z(z) {}
    inline constexpr explicit Vector3(float xyz) noexcept : x(xyz), y(xyz), z(xyz) {}
#pragma endregion
#pragma region 演算子のオーバーロード
    inline constexpr const float& operator[](std::size_t index) const noexcept {
        return *(&x + index);
    }
    inline constexpr float& operator[](std::size_t index) noexcept {
        return const_cast<float&>(static_cast<const Vector3&>(*this)[index]);
    }

    inline explicit operator Vector2() const {
        return { x,y };
    }

    friend inline constexpr Vector3 operator+(const Vector3& rhs) noexcept {
        return rhs;
    }
    friend inline constexpr Vector3 operator-(const Vector3& rhs) noexcept {
        return { -rhs.x, -rhs.y, -rhs.z };
    }
    friend inline constexpr Vector3 operator+(const Vector3& lhs, const Vector3& rhs) noexcept {
        return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
    }
    friend inline constexpr Vector3 operator-(const Vector3& lhs, const Vector3& rhs) noexcept {
        return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
    }
    friend inline constexpr Vector3 operator*(const Vector3& lhs, float rhs) noexcept {
        return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
    }
    friend inline constexpr Vector3 operator*(float lhs, const Vector3& rhs) noexcept {
        return { lhs * rhs.x, lhs * rhs.y, lhs * rhs.z };
    }
    friend inline constexpr Vector3 operator/(const Vector3& lhs, float rhs) noexcept {
        return lhs * (1.0f / rhs);
    }
    friend inline constexpr Vector3& operator+=(Vector3& lhs, const Vector3& rhs) noexcept {
        lhs = lhs + rhs;
        return lhs;
    }
    friend inline constexpr Vector3& operator-=(Vector3& lhs, const Vector3& rhs) noexcept {
        lhs = lhs - rhs;
        return lhs;
    }
    friend inline constexpr Vector3& operator*=(Vector3& lhs, float rhs) noexcept {
        lhs = lhs * rhs;
        return lhs;
    }
    friend inline constexpr Vector3& operator/=(Vector3& lhs, float rhs) noexcept {
        lhs = lhs / rhs;
        return lhs;
    }
    friend inline constexpr float Dot(const Vector3& lhs, const Vector3& rhs) noexcept {
        return Vector3::Dot(lhs, rhs);
    }
    friend inline constexpr Vector3 Cross(const Vector3& lhs, const Vector3& rhs) noexcept {
        return Vector3::Cross(lhs, rhs);
    }
    friend inline constexpr bool operator==(const Vector3& lhs, const Vector3& rhs) noexcept {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
    }
    friend inline constexpr bool operator!=(const Vector3& lhs, const Vector3& rhs) noexcept {
        return !(lhs == rhs);
    }
#pragma endregion
#pragma region メンバ関数
    inline constexpr Vector3& SetXY(const Vector2& xy) noexcept {
        x = xy.x, y = xy.y;
        return *this;
    }
    inline constexpr Vector2 GetXY() const noexcept {
        return { x, y };
    }
    inline constexpr Vector3& SetXZ(const Vector2& xz) noexcept {
        x = xz.x, z = xz.y;
        return *this;
    }
    inline constexpr Vector2 GetXZ() const noexcept {
        return { x, z };
    }
    inline constexpr Vector3& SetYZ(const Vector2& yz) noexcept {
        y = yz.x, z = yz.y;
        return *this;
    }
    inline constexpr Vector2 GetYZ() const noexcept {
        return { y, z };
    }

    inline constexpr float LengthSquare() const noexcept {
        return x * x + y * y + z * z;
    }
    inline float Length() const noexcept {
        return std::sqrt(LengthSquare());
    }
    inline Vector3 Normalized() const noexcept {
        return *this / Length();
    }
    inline Vector3& Normalize() noexcept {
        return (*this = Normalized());
    }
#pragma endregion
#pragma region 静的関数
    static inline float Angle(const Vector3& from, const Vector3& to) noexcept {
        return std::acos(Dot(from.Normalized(), to.Normalized()));
    }
    static inline float SignedAngle(const Vector3& from, const Vector3& to, const Vector3& axis) noexcept {
        return Dot(Cross(from, to), axis) < 0.0f ? Angle(from, to) : -Angle(from, to);
    }
    static inline float Distance(const Vector3& start, const Vector3& end) noexcept {
        return (end - start).Length();
    }
    static inline constexpr float Dot(const Vector3& lhs, const Vector3& rhs) noexcept {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
    }
    static inline constexpr Vector3 Cross(const Vector3& lhs, const Vector3& rhs) noexcept {
        return {
                lhs.y * rhs.z - lhs.z * rhs.y,
                lhs.z * rhs.x - lhs.x * rhs.z,
                lhs.x * rhs.y - lhs.y * rhs.x };
    }
    static inline constexpr Vector3 Scale(const Vector3& lhs, const Vector3& rhs) noexcept {
        return { lhs.x * rhs.x , lhs.y * rhs.y, lhs.z * rhs.z };
    }
    static inline constexpr Vector3 Project(const Vector3& base, const Vector3& direction) noexcept {
        return Dot(base, direction) * direction;
    }
    static inline constexpr Vector3 ProjectOnPlane(const Vector3& base, const Vector3& planeNormal) noexcept {
        return base - planeNormal * Dot(base, planeNormal);
    }
    static inline constexpr Vector3 Reflecte(const Vector3& direction, const Vector3& normal) noexcept {
        return Dot(normal, -direction) * 2.0f * normal + direction;
    }
    static inline constexpr Vector3 Min(const Vector3& lhs, const Vector3& rhs) noexcept {
        return { std::min(lhs.x,rhs.x), std::min(lhs.y,rhs.y), std::min(lhs.z,rhs.z) };
    }
    static inline constexpr Vector3 Max(const Vector3& lhs, const Vector3& rhs) noexcept {
        return { std::max(lhs.x,rhs.x), std::max(lhs.y,rhs.y), std::max(lhs.z,rhs.z) };
    }
    static inline constexpr Vector3 Clamp(const Vector3& value, const Vector3& min, const Vector3& max) noexcept {
        return { std::clamp(value.x, min.x, max.x), std::clamp(value.y, min.y, max.y), std::clamp(value.z, min.z, max.z) };
    }
    static inline constexpr Vector3 Lerp(float t, const Vector3& start, const Vector3& end) noexcept {
        return start + t * (end - start);
    }
    static Vector3 Slerp(float t, const Vector3& start, const Vector3& end) noexcept;
    static Vector3 CatmullRomSpline(float t, const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3) noexcept;
    static Vector3 QuadraticBezierCurve(float t, const Vector3& p0, const Vector3& p1, const Vector3& p2) noexcept;
    static Vector3 CubicBezierCurve(float t, const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3) noexcept;
#pragma endregion
#pragma region メンバ変数
    float x, y, z;
#pragma endregion
};

class Vector4 {
public:
#pragma region 定数
    static const Vector4 zero;
    static const Vector4 one;
#pragma endregion
#pragma region コンストラクタ
    inline constexpr Vector4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    inline constexpr Vector4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
    inline constexpr Vector4(const Vector3& xyz, float w) noexcept : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
    inline constexpr explicit Vector4(float xyzw) noexcept : x(xyzw), y(xyzw), z(xyzw), w(xyzw) {}
#pragma endregion
#pragma region 演算子のオーバーロード
    inline constexpr const float& operator[](std::size_t index) const noexcept {
        return *(&x + index);
    }
    inline constexpr float& operator[](std::size_t index) noexcept {
        return const_cast<float&>(static_cast<const Vector4&>(*this)[index]);
    }
    inline constexpr explicit operator Vector3() const noexcept {
        return { x, y, z };
    }

    friend inline constexpr Vector4 operator+(const Vector4& rhs) noexcept {
        return rhs;
    }
    friend inline constexpr Vector4 operator-(const Vector4& rhs) noexcept {
        return { -rhs.x, -rhs.y, -rhs.z, -rhs.w };
    }
    friend inline constexpr Vector4 operator+(const Vector4& lhs, const Vector4& rhs) noexcept {
        return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w };
    }
    friend inline constexpr Vector4 operator-(const Vector4& lhs, const Vector4& rhs) noexcept {
        return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w };
    }
    friend inline constexpr Vector4 operator*(const Vector4& lhs, float rhs) noexcept {
        return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs };
    }
    friend inline constexpr Vector4 operator*(float lhs, const Vector4& rhs) noexcept {
        return { lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w };
    }
    friend inline constexpr Vector4 operator/(const Vector4& lhs, float rhs) noexcept {
        return lhs * (1.0f / rhs);
    }
    friend inline constexpr Vector4& operator+=(Vector4& lhs, const Vector4& rhs) noexcept {
        lhs = lhs + rhs;
        return lhs;
    }
    friend inline constexpr Vector4& operator-=(Vector4& lhs, const Vector4& rhs) noexcept {
        lhs = lhs - rhs;
        return lhs;
    }
    friend inline constexpr Vector4& operator*=(Vector4& lhs, float rhs) noexcept {
        lhs = lhs * rhs;
        return lhs;
    }
    friend inline constexpr Vector4& operator/=(Vector4& lhs, float rhs) noexcept {
        lhs = lhs / rhs;
        return lhs;
    }
    friend inline constexpr float Dot(const Vector4& lhs, const Vector4& rhs) noexcept {
        return Vector4::Dot(lhs, rhs);
    }
    friend inline constexpr bool operator==(const Vector4& lhs, const Vector4& rhs) noexcept {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
    }
    friend inline constexpr bool operator!=(const Vector4& lhs, const Vector4& rhs) noexcept {
        return !(lhs == rhs);
    }
#pragma endregion
#pragma region メンバ関数
    inline constexpr Vector4& SetXYZ(const Vector3& xyz) noexcept {
        x = xyz.x, y = xyz.y, z = xyz.z;
        return *this;
    }
    inline constexpr Vector3 GetXYZ() const noexcept {
        return { x, y, z };
    }

    inline constexpr float LengthSquare() const noexcept {
        return x * x + y * y + z * z + w * w;
    }
    inline float Length() const noexcept {
        return std::sqrt(LengthSquare());
    }
    inline Vector4 Normalized() const noexcept {
        return *this / Length();
    }
    inline Vector4& Normalize() noexcept {
        return (*this = Normalized());
    }
#pragma endregion
#pragma region 静的関数
    static inline constexpr float Dot(const Vector4& lhs, const Vector4& rhs) noexcept {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
    }
    static inline constexpr Vector4 Scale(const Vector4& lhs, const Vector4& rhs) noexcept {
        return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.z * rhs.z };
    }
    static inline Vector4 Project(const Vector4& base, const Vector4& direction) noexcept {
        return Dot(base, direction) * direction;
    }
    static inline constexpr Vector4 Min(const Vector4& lhs, const Vector4& rhs) noexcept {
        return { std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y), std::min(lhs.z, rhs.z), std::min(lhs.w, rhs.w) };
    }
    static inline constexpr Vector4 Max(const Vector4& lhs, const Vector4& rhs) noexcept {
        return { std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y), std::max(lhs.z, rhs.z), std::max(lhs.w, rhs.w) };
    }
    static inline constexpr Vector4 Clamp(const Vector4& value, const Vector4& min, const Vector4& max) noexcept {
        return { std::clamp(value.x, min.x, max.x), std::clamp(value.y, min.y, max.y), std::clamp(value.z, min.z, max.z), std::clamp(value.w, min.w, max.w) };
    }
    static inline constexpr Vector4 Lerp(float t, const Vector4& start, const Vector4& end) noexcept {
        return start + t * (end - start);
    }
#pragma endregion
#pragma region メンバ変数
    float x, y, z, w;
#pragma endregion 
};

class Quaternion {
public:
#pragma region 定数
    static const Quaternion identity;
#pragma endregion
#pragma region ファクトリ関数
    static inline Quaternion MakeFromAngleAxis(float angle, const Vector3& axis) noexcept {
        Vector3 v = axis.Normalized() * std::sin(angle * 0.5f);
        return Quaternion{
            v.x,
            v.y,
            v.z,
            std::cos(angle * 0.5f) };
    }
    static inline Quaternion MakeFromEulerAngle(const Vector3& euler) noexcept {
        Vector3 s = Vector3(std::sin(euler.x * 0.5f), std::sin(euler.y * 0.5f), std::sin(euler.z * 0.5f));
        Vector3 c = Vector3(std::cos(euler.x * 0.5f), std::cos(euler.y * 0.5f), std::cos(euler.z * 0.5f));
        return Quaternion{
            s.x * c.y * c.z - c.x * s.y * s.z,
            c.x * s.y * c.z + s.x * c.y * s.z,
            c.x * c.y * s.z - s.x * s.y * c.z,
            c.x * c.y * c.z + s.x * s.y * s.z };
    }
    static inline Quaternion MakeForXAxis(float angle) noexcept {
        return Quaternion(std::sin(angle / 2.0f), 0.0f, 0.0f, std::cos(angle / 2.0f));
    }
    static inline Quaternion MakeForYAxis(float angle) noexcept {
        return Quaternion(0.0f, std::sin(angle / 2.0f), 0.0f, std::cos(angle / 2.0f));
    }
    static inline Quaternion MakeForZAxis(float angle) noexcept {
        return Quaternion(0.0f, 0.0f, std::sin(angle / 2.0f), std::cos(angle / 2.0f));
    }

    // 二つのベクトルに垂直な軸と差分角から生成
    static inline Quaternion MakeFromTwoVector(const Vector3& from, const Vector3& to) noexcept {
        Vector3 axis = Vector3::Cross(from, to).Normalized();
        float angle = Vector3::Angle(from, to);
        return MakeFromAngleAxis(angle, axis);
    }
    static Quaternion MakeFromOrthonormal(const Vector3& x, const Vector3& y, const Vector3& z) noexcept;
    static inline Quaternion MakeLookRotation(const Vector3& direction, const Vector3& up = Vector3::up) noexcept {
        Vector3 z = direction.Normalized();
        Vector3 x = Vector3::Cross(up, z).Normalized();
        Vector3 y = Vector3::Cross(z, x);
        return MakeFromOrthonormal(x, y, z);
    }
#pragma endregion
#pragma region コンストラクタ
    inline constexpr Quaternion() noexcept : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    inline constexpr explicit Quaternion(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
#pragma endregion
#pragma region 演算子のオーバーロード
    inline constexpr const float& operator[](size_t index) const noexcept {
        return *(&x + index);
    }
    inline constexpr float& operator[](size_t index) noexcept {
        return const_cast<float&>(static_cast<const Quaternion&>(*this)[index]);
    }

    friend inline constexpr Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs) noexcept {
        return Quaternion{ lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w };
    }
    friend inline constexpr Quaternion operator*(const Quaternion& lhs, float rhs) noexcept {
        return Quaternion{ lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs };
    }
    friend inline constexpr Quaternion operator*(float lhs, const Quaternion& rhs) noexcept {
        return Quaternion{ lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w };
    }
    friend inline constexpr Vector3 operator*(const Quaternion& lhs, const Vector3& rhs) noexcept {
        float x = lhs.w * rhs.x + lhs.y * rhs.z - lhs.z * rhs.y;
        float y = lhs.w * rhs.y + lhs.z * rhs.x - lhs.x * rhs.z;
        float z = lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x;
        float w = -lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;

        return {
            x * lhs.w + w * -lhs.x + y * -lhs.z - z * -lhs.y,
            y * lhs.w + w * -lhs.y + z * -lhs.x - x * -lhs.z,
            z * lhs.w + w * -lhs.z + x * -lhs.y - y * -lhs.x,
        };
        // return rhs + 2.0f * Vector3::Cross({ lhs.x, lhs.y, lhs.z }, Vector3::Cross({ lhs.x, lhs.y, lhs.z }, rhs) + lhs.w * rhs);
    }
    friend inline constexpr Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs) noexcept {
        return Quaternion{
            lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x,
            lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z };
    }
    friend inline constexpr Quaternion& operator*=(Quaternion& lhs, const Quaternion& rhs) noexcept {
        lhs = lhs * rhs;
        return lhs;
    }
    friend inline constexpr bool operator==(const Quaternion& lhs, const Quaternion& rhs) noexcept {
        return Dot(lhs, rhs) > 0.99999f;
    }
    friend inline constexpr bool operator!=(const Quaternion& lhs, const Quaternion& rhs) noexcept {
        return !(lhs == rhs);
    }
#pragma endregion
#pragma region メンバ関数
    inline Quaternion Concatenate(const Quaternion& rot) {
        return rot * (*this);
    }
    // オイラー角（不安）
    inline Vector3 EulerAngle() const noexcept {
        Vector3 euler{};
        euler.x = std::atan2(2.0f * (w * x + y * z), 1.0f - 2.0f * (x * x + y * y));
        float s = 2.0f * (w * y - z * x);
        if (std::abs(s) >= 1.0f) {
            euler.y = std::copysign(Math::HalfPi, s);
        }
        else {
            euler.y = std::asin(s);
        }
        euler.z = std::atan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (y * y + z * z));
        return euler;
    }
    inline constexpr float LengthSquare() const noexcept {
        return x * x + y * y + z * z + w * w;
    }
    inline float Length() const noexcept {
        return std::sqrt(LengthSquare());
    }
    inline Quaternion Normalized() const noexcept {
        return *this * (1.0f / Length());
    }
    inline float Angle() const noexcept {
        return std::acos(w) * 2.0f;
    }
    inline Vector3 Axis() const noexcept {
        return Vector3{ x, y, z } *(1.0f / std::sin(std::acos(w)));
    }
    inline constexpr Quaternion Conjugate() const noexcept {
        return Quaternion{ -x, -y, -z, w };
    }
    inline Quaternion Inverse() const noexcept {
        return Conjugate() * (1.0f / Length());
    }
    inline constexpr Vector3 GetRight() const noexcept {
        float yw = y * w, zw = z * w;
        float yx = y * x, zx = z * x;
        return {
             w * w + x * x - y * y - z * z,
             yx + yx + zw + zw,
             zx + zx - yw - yw,
        };
    }
    inline constexpr Vector3 GetUp() const noexcept {
        float xw = x * w, zw = z * w;
        float xy = x * y, zy = z * y;
        return {
             xy + xy - zw - zw,
             w * w - x * x + y * y - z * z,
             zy + zy + xw + xw,
        };
    }
    inline constexpr Vector3 GetForward() const noexcept {
        float xw = x * w, yw = y * w;
        float xz = x * z, yz = y * z;
        return {
             xz + xz + yw + yw,
             yz + yz - xw - xw,
             w * w - x * x - y * y + z * z,
        };
    }
#pragma endregion
#pragma region 静的関数
    static inline constexpr float Dot(const Quaternion& lhs, const Quaternion& rhs) noexcept {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
    }
    static inline constexpr Quaternion Lerp(float t, const Quaternion& start, const Quaternion& end) noexcept {
        return Quaternion{
            start.x + t * (end.x - start.x),
            start.y + t * (end.y - start.y),
            start.z + t * (end.z - start.z),
            start.w + t * (end.w - start.w) };
    }
    static Quaternion Slerp(float t, const Quaternion& start, const Quaternion& end) noexcept;
#pragma endregion 
#pragma region メンバ変数
    float x, y, z, w;
#pragma endregion
};

class Matrix3x3 {
public:
    static const Matrix3x3 identity;

    static inline Matrix3x3 MakeScaling(const Vector2& scale) noexcept {
        return {
            scale.x, 0.0f, 0.0f,
            0.0f, scale.y, 0.0f,
            0.0f, 0.0f, 1.0f };
    }

    static inline Matrix3x3 MakeRotation(float rotate) noexcept {
        float s = std::sin(rotate);
        float c = std::cos(rotate);
        return {
            c,		s,		0.0f,
            -s,		c,		0.0f,
            0.0f,	0.0f,	1.0f };
    }
    static inline constexpr Matrix3x3 MakeTranslation(const Vector2& translate) noexcept {
        return {
            1.0f,		0.0f,		0.0f,
            0.0f,		1.0f,		0.0f,
            translate.x,	translate.y,	1.0f };
    }
    static inline Matrix3x3 MakeAffineTransform(const Vector2& scale, float rotate, const Vector2& translate) noexcept {
        float s = std::sin(rotate);
        float c = std::cos(rotate);
        return {
                scale.x * c,
                scale.x * s,
                0.0f,

                scale.y * -s,
                scale.y * c,
                0.0f,

                translate.x,
                translate.y,
                1.0f };
    }

    inline constexpr Matrix3x3() noexcept {
        *this = identity;
    }
    inline constexpr Matrix3x3(
        float _00, float _01, float _02,
        float _10, float _11, float _12,
        float _20, float _21, float _22) noexcept {
        m[0][0] = _00, m[0][1] = _01, m[0][2] = _02;
        m[1][0] = _10, m[1][1] = _11, m[1][2] = _12;
        m[2][0] = _20, m[2][1] = _21, m[2][2] = _22;
    }

    friend inline constexpr Matrix3x3 operator*(const Matrix3x3& lhs, const Matrix3x3& rhs) noexcept {
        return {
             lhs.m[0][0] * rhs.m[0][0] + lhs.m[0][1] * rhs.m[1][0] + lhs.m[0][2] * rhs.m[2][0],
             lhs.m[0][0] * rhs.m[0][1] + lhs.m[0][1] * rhs.m[1][1] + lhs.m[0][2] * rhs.m[2][1],
             lhs.m[0][0] * rhs.m[0][2] + lhs.m[0][1] * rhs.m[1][2] + lhs.m[0][2] * rhs.m[2][2],

             lhs.m[1][0] * rhs.m[0][0] + lhs.m[1][1] * rhs.m[1][0] + lhs.m[1][2] * rhs.m[2][0],
             lhs.m[1][0] * rhs.m[0][1] + lhs.m[1][1] * rhs.m[1][1] + lhs.m[1][2] * rhs.m[2][1],
             lhs.m[1][0] * rhs.m[0][2] + lhs.m[1][1] * rhs.m[1][2] + lhs.m[1][2] * rhs.m[2][2],

             lhs.m[2][0] * rhs.m[0][0] + lhs.m[2][1] * rhs.m[1][0] + lhs.m[2][2] * rhs.m[2][0],
             lhs.m[2][0] * rhs.m[0][1] + lhs.m[2][1] * rhs.m[1][1] + lhs.m[2][2] * rhs.m[2][1],
             lhs.m[2][0] * rhs.m[0][2] + lhs.m[2][1] * rhs.m[1][2] + lhs.m[2][2] * rhs.m[2][2]
        };
    }
    friend inline constexpr Vector2 operator*(const Vector2& lhs, const Matrix3x3& rhs) noexcept {
        return {
                lhs.x * rhs.m[0][0] + lhs.y * rhs.m[1][0] + rhs.m[2][0],
                lhs.x * rhs.m[0][1] + lhs.y * rhs.m[1][1] + rhs.m[2][1] };
    }
    friend inline constexpr Vector3 operator*(const Vector3& lhs, const Matrix3x3& rhs) noexcept {
        return {
                lhs.x * rhs.m[0][0] + lhs.y * rhs.m[1][0] + lhs.z * rhs.m[2][0],
                lhs.x * rhs.m[0][1] + lhs.y * rhs.m[1][1] + lhs.z * rhs.m[2][1],
                lhs.x * rhs.m[0][2] + lhs.y * rhs.m[1][2] + lhs.z * rhs.m[2][2] };
    }
    friend inline constexpr Matrix3x3 operator*(float lhs, const Matrix3x3& rhs) noexcept {
        return {
            lhs * rhs.m[0][0], lhs * rhs.m[0][1], lhs * rhs.m[0][2] ,
            lhs * rhs.m[1][0], lhs * rhs.m[1][1], lhs * rhs.m[1][2] ,
            lhs * rhs.m[2][0], lhs * rhs.m[2][1], lhs * rhs.m[2][2] };
    }
    friend inline constexpr Matrix3x3 operator*(const Matrix3x3& lhs, float rhs) noexcept {
        return {
            lhs.m[0][0] * rhs, lhs.m[0][1] * rhs, lhs.m[0][2] * rhs,
            lhs.m[1][0] * rhs, lhs.m[1][1] * rhs, lhs.m[1][2] * rhs,
            lhs.m[2][0] * rhs, lhs.m[2][1] * rhs, lhs.m[2][2] * rhs, };
    }
    friend inline constexpr Matrix3x3& operator*=(Matrix3x3& lhs, const Matrix3x3& rhs) noexcept {
        lhs = lhs * rhs;
        return lhs;
    }
    // 行
    inline constexpr Matrix3x3& SetRow(size_t i, const Vector3& v) noexcept {
        m[i][0] = v.x, m[i][1] = v.y, m[i][2] = v.z;
        return *this;
    }
    // 行
    inline constexpr Vector3 GetRow(size_t i) const noexcept {
        return { m[i][0], m[i][1], m[i][2] };
    }
    // 列
    inline constexpr Matrix3x3& SetColumn(size_t i, const Vector3& v) noexcept {
        m[0][i] = v.x, m[1][i] = v.y, m[2][i] = v.z;
        return *this;
    }
    // 列
    inline constexpr Vector3 GetColumn(size_t i) const noexcept {
        return { m[0][i], m[1][i], m[2][i] };
    }
    inline constexpr Matrix3x3& SetXAxis(const Vector2& v) noexcept {
        m[0][0] = v.x, m[0][1] = v.y;
        return *this;
    }
    inline constexpr Vector2 GetXAxis() const noexcept {
        return { m[0][0], m[0][1] };
    }
    inline constexpr Matrix3x3& SetYAxis(const Vector2& v) noexcept {
        m[1][0] = v.x, m[1][1] = v.y;
        return *this;
    }
    inline constexpr Vector2 GetYAxis() const noexcept {
        return { m[1][0], m[1][1] };
    }
    inline constexpr Matrix3x3& SetZAxis(const Vector2& v) noexcept {
        m[2][0] = v.x, m[2][1] = v.y;
        return *this;
    }
    inline constexpr Vector2 GetZAxis() const noexcept {
        return { m[2][0], m[2][1] };
    }
    inline constexpr Matrix3x3& SetTranslate(const Vector2& v) noexcept {
        m[2][0] = v.x, m[2][1] = v.y;
        return *this;
    }
    inline constexpr Vector2 GetTranslate() const noexcept {
        return { m[2][0], m[2][1] };
    }
    inline float Determinant() const noexcept {
        return m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0] + m[0][2] * m[1][0] * m[2][1]
            - m[0][2] * m[1][1] * m[2][0] - m[0][1] * m[1][0] * m[2][2] - m[0][0] * m[1][2] * m[2][1];
    }
    inline Matrix3x3 Adjugate() const noexcept {
        return {
              m[1][1] * m[2][2] - m[1][2] * m[2][1],
            -(m[0][1] * m[2][2] - m[0][2] * m[2][1]),
              m[0][1] * m[1][2] - m[0][2] * m[1][1],

            -(m[1][0] * m[2][2] - m[1][2] * m[2][0]),
              m[0][0] * m[2][2] - m[0][2] * m[2][0],
            -(m[0][0] * m[1][2] - m[0][2] * m[1][0]),

              m[1][0] * m[2][1] - m[1][1] * m[2][0],
            -(m[0][0] * m[2][1] - m[0][1] * m[2][0]),
              m[0][0] * m[1][1] - m[0][1] * m[1][0] };
    }
    inline Matrix3x3 Inverse() const noexcept {
        return 1.0f / Determinant() * Adjugate();
    }
    inline constexpr Matrix3x3 Transpose() const noexcept {
        return {
            m[0][0], m[1][0], m[2][0],
            m[0][1], m[1][1], m[2][1],
            m[0][2], m[1][2], m[2][2] };
    }
    float m[3][3];
};

class Matrix4x4 {
public:
#pragma region 定数
    static const Matrix4x4 identity;
#pragma endregion
#pragma region ファクトリ関数
    static inline Matrix4x4 MakeScaling(const Vector3& scale) noexcept {
        return {
            scale.x, 0.0f, 0.0f, 0.0f,
            0.0f, scale.y, 0.0f, 0.0f,
            0.0f, 0.0f, scale.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f };
    }
    static inline Matrix4x4 MakeRotationX(float rotate) noexcept {
        float c = std::cos(rotate);
        float s = std::sin(rotate);
        return {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, c, s, 0.0f,
            0.0f, -s, c, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f };
    }
    static inline Matrix4x4 MakeRotationY(float rotate) noexcept {
        float s = std::sin(rotate);
        float c = std::cos(rotate);
        return {
            c,		0.0f,	-s,		0.0f,
            0.0f,	1.0f,	0.0f,	0.0f,
            s,		0.0f,	c,		0.0f,
            0.0f,	0.0f,	0.0f,	1.0f };
    }
    static inline Matrix4x4 MakeRotationZ(float rotate) noexcept {
        float s = std::sin(rotate);
        float c = std::cos(rotate);
        return {
            c,		s,		0.0f,	0.0f,
            -s,		c,		0.0f,	0.0f,
            0.0f,	0.0f,	1.0f,	0.0f,
            0.0f,	0.0f,	0.0f,	1.0f };
    }
    static inline Matrix4x4 MakeRotationXYZ(const Vector3& rotate) noexcept {
        Vector3 s = { std::sin(rotate.x), std::sin(rotate.y), std::sin(rotate.z) };
        Vector3 c = { std::cos(rotate.x), std::cos(rotate.y), std::cos(rotate.z) };
        return {
            c.y * c.z,						c.y * s.z,						-s.y,		0.0f,
            s.x * s.y * c.z - c.x * s.z,	s.x * s.y * s.z + c.x * c.z,	s.x * c.y,	0.0f,
            c.x * s.y * c.z + s.x * s.z,	c.x * s.y * s.z - s.x * c.z,	c.x * c.y,	0.0f,
            0.0f,	0.0f,	0.0f,	1.0f };
    }
    static inline constexpr Matrix4x4 MakeRotation(const Quaternion& q) noexcept {
        float w2 = q.w * q.w, x2 = q.x * q.x, y2 = q.y * q.y, z2 = q.z * q.z;
        float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
        float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
        return {
            w2 + x2 - y2 - z2,	2.0f * (wz + xy),	2.0f * (xz - wy),	0.0f,
            2.0f * (xy - wz),	w2 - x2 + y2 - z2,	2.0f * (yz + wx),	0.0f,
            2.0f * (wy + xz),	2.0f * (-wx + yz),	w2 - x2 - y2 + z2,	0.0f,
            0.0f,				0.0f,				0.0f,				1.0f };
    }
    static inline Matrix4x4 MakeLookRotation(const Vector3& direction, const Vector3& up = Vector3::up) noexcept {
        Vector3 z = direction.Normalized();
        Vector3 x = Vector3::Cross(up.Normalized(), z).Normalized();
        Vector3 y = Vector3::Cross(z, x);
        return {
            x.x, x.y, x.z, 0.0f,
            y.x, y.y, y.z, 0.0f,
            z.x, z.y, z.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f };
    }
    static inline constexpr Matrix4x4 MakeTranslation(const Vector3& translate) noexcept {
        return {
            1.0f,		0.0f,		0.0f,		0.0f,
            0.0f,		1.0f,		0.0f,		0.0f,
            0.0f,		0.0f,		1.0f,		0.0f,
            translate.x,	translate.y,	translate.z,	1.0f };
    }
    static inline Matrix4x4 MakeAffineTransform(const Vector3& scale, const Vector3& rotate, const Vector3& translate) noexcept {
        Vector3 s = { std::sin(rotate.x), std::sin(rotate.y), std::sin(rotate.z) };
        Vector3 c = { std::cos(rotate.x), std::cos(rotate.y), std::cos(rotate.z) };
        return {
                scale.x * (c.y * c.z),
                scale.x * (c.y * s.z),
                scale.x * (-s.y),
                0.0f,

                scale.y * (s.x * s.y * c.z - c.x * s.z),
                scale.y * (s.x * s.y * s.z + c.x * c.z),
                scale.y * (s.x * c.y),
                0.0f,

                scale.z * (c.x * s.y * c.z + s.x * s.z),
                scale.z * (c.x * s.y * s.z - s.x * c.z),
                scale.z * (c.x * c.y),
                0.0f,

                translate.x,
                translate.y,
                translate.z,
                1.0f };
    }
    static inline constexpr Matrix4x4 MakeAffineTransform(const Vector3& scale, const Quaternion& rotate, const Vector3& translate) noexcept {
        float w2 = rotate.w * rotate.w, x2 = rotate.x * rotate.x, y2 = rotate.y * rotate.y, z2 = rotate.z * rotate.z;
        float wx = rotate.w * rotate.x, wy = rotate.w * rotate.y, wz = rotate.w * rotate.z;
        float xy = rotate.x * rotate.y, xz = rotate.x * rotate.z, yz = rotate.y * rotate.z;
        return {
                scale.x * (w2 + x2 - y2 - z2),
                scale.x * (2.0f * (wz + xy)),
                scale.x * (2.0f * (xz - wy)),
                0.0f,

                scale.y * (2.0f * (xy - wz)),
                scale.y * (w2 - x2 + y2 - z2),
                scale.y * (2.0f * (yz + wx)),
                0.0f,

                scale.z * (2.0f * (wy + xz)),
                scale.z * (2.0f * (-wx + yz)),
                scale.z * (w2 - x2 - y2 + z2),
                0.0f,

                translate.x,
                translate.y,
                translate.z,
                1.0f };
    }
    static inline Matrix4x4 MakeAffineInverse(const Matrix4x4& rotate, const Vector3& translate) noexcept {
        Matrix4x4 rt = rotate.Transpose();
        return rt.SetTranslate(-translate * rt);
    }
    static inline Matrix4x4 MakePerspectiveProjection(float fovY, float aspect, float nearZ, float farZ) noexcept {
        float s = 1.0f / std::tan(fovY * 0.5f);
        float a = farZ / (farZ - nearZ);
        return {
            s / aspect, 0.0f, 0.0f, 0.0f,
            0.0f, s, 0.0f, 0.0f,
            0.0f, 0.0f, a, 1.0f,
            0.0f, 0.0f, a * -nearZ, 0.0f };
    }
    static inline constexpr Matrix4x4 MakeOrthographicProjection(float width, float height, float nearZ, float farZ) noexcept {
        float zRange = farZ - nearZ;
        return {
            2.0f / width, 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / height, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f / zRange, 0.0f,
            0.0f, 0.0f, nearZ / -zRange, 1.0f };
    }
    static inline constexpr Matrix4x4 MakeViewport(float left, float top, float width, float height, float nearZ, float farZ) noexcept {
        float halfW = width / 2.0f;
        float halfh = height / 2.0f;
        return {
            halfW, 0.0f, 0.0f, 0.0f,
            0.0f, -halfh, 0.0f, 0.0f,
            0.0f, 0.0f, farZ - nearZ, 0.0f,
            left + halfW, top + halfh, nearZ, 1.0f };
    }
#pragma endregion
#pragma region コンストラクタ
    inline constexpr Matrix4x4() noexcept {
        *this = identity;
    }
    inline constexpr Matrix4x4(
        float _00, float _01, float _02, float _03,
        float _10, float _11, float _12, float _13,
        float _20, float _21, float _22, float _23,
        float _30, float _31, float _32, float _33) noexcept {
        m[0][0] = _00, m[0][1] = _01, m[0][2] = _02, m[0][3] = _03;
        m[1][0] = _10, m[1][1] = _11, m[1][2] = _12, m[1][3] = _13;
        m[2][0] = _20, m[2][1] = _21, m[2][2] = _22, m[2][3] = _23;
        m[3][0] = _30, m[3][1] = _31, m[3][2] = _32, m[3][3] = _33;
    }
#pragma endregion
#pragma region 演算子のオーバーロード
    friend inline Matrix4x4 operator*(const Matrix4x4& lhs, const Matrix4x4& rhs) noexcept {

        DirectX::XMMATRIX lhsMatrix = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(lhs.m));
        DirectX::XMMATRIX rhsMatrix = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(rhs.m));
        DirectX::XMMATRIX mulMatrix = DirectX::XMMatrixMultiply(lhsMatrix, rhsMatrix);

        Matrix4x4 result;
        DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(result.m), mulMatrix);
        return result;
        
        //return {
        //    lhs.m[0][0] * rhs.m[0][0] + lhs.m[0][1] * rhs.m[1][0] + lhs.m[0][2] * rhs.m[2][0] + lhs.m[0][3] * rhs.m[3][0],
        //    lhs.m[0][0] * rhs.m[0][1] + lhs.m[0][1] * rhs.m[1][1] + lhs.m[0][2] * rhs.m[2][1] + lhs.m[0][3] * rhs.m[3][1],
        //    lhs.m[0][0] * rhs.m[0][2] + lhs.m[0][1] * rhs.m[1][2] + lhs.m[0][2] * rhs.m[2][2] + lhs.m[0][3] * rhs.m[3][2],
        //    lhs.m[0][0] * rhs.m[0][3] + lhs.m[0][1] * rhs.m[1][3] + lhs.m[0][2] * rhs.m[2][3] + lhs.m[0][3] * rhs.m[3][3],
        //
        //    lhs.m[1][0] * rhs.m[0][0] + lhs.m[1][1] * rhs.m[1][0] + lhs.m[1][2] * rhs.m[2][0] + lhs.m[1][3] * rhs.m[3][0],
        //    lhs.m[1][0] * rhs.m[0][1] + lhs.m[1][1] * rhs.m[1][1] + lhs.m[1][2] * rhs.m[2][1] + lhs.m[1][3] * rhs.m[3][1],
        //    lhs.m[1][0] * rhs.m[0][2] + lhs.m[1][1] * rhs.m[1][2] + lhs.m[1][2] * rhs.m[2][2] + lhs.m[1][3] * rhs.m[3][2],
        //    lhs.m[1][0] * rhs.m[0][3] + lhs.m[1][1] * rhs.m[1][3] + lhs.m[1][2] * rhs.m[2][3] + lhs.m[1][3] * rhs.m[3][3],
        //
        //    lhs.m[2][0] * rhs.m[0][0] + lhs.m[2][1] * rhs.m[1][0] + lhs.m[2][2] * rhs.m[2][0] + lhs.m[2][3] * rhs.m[3][0],
        //    lhs.m[2][0] * rhs.m[0][1] + lhs.m[2][1] * rhs.m[1][1] + lhs.m[2][2] * rhs.m[2][1] + lhs.m[2][3] * rhs.m[3][1],
        //    lhs.m[2][0] * rhs.m[0][2] + lhs.m[2][1] * rhs.m[1][2] + lhs.m[2][2] * rhs.m[2][2] + lhs.m[2][3] * rhs.m[3][2],
        //    lhs.m[2][0] * rhs.m[0][3] + lhs.m[2][1] * rhs.m[1][3] + lhs.m[2][2] * rhs.m[2][3] + lhs.m[2][3] * rhs.m[3][3],
        //
        //    lhs.m[3][0] * rhs.m[0][0] + lhs.m[3][1] * rhs.m[1][0] + lhs.m[3][2] * rhs.m[2][0] + lhs.m[3][3] * rhs.m[3][0],
        //    lhs.m[3][0] * rhs.m[0][1] + lhs.m[3][1] * rhs.m[1][1] + lhs.m[3][2] * rhs.m[2][1] + lhs.m[3][3] * rhs.m[3][1],
        //    lhs.m[3][0] * rhs.m[0][2] + lhs.m[3][1] * rhs.m[1][2] + lhs.m[3][2] * rhs.m[2][2] + lhs.m[3][3] * rhs.m[3][2],
        //    lhs.m[3][0] * rhs.m[0][3] + lhs.m[3][1] * rhs.m[1][3] + lhs.m[3][2] * rhs.m[2][3] + lhs.m[3][3] * rhs.m[3][3] };
    }
    friend inline constexpr Vector3 operator*(const Vector3& lhs, const Matrix4x4& rhs) noexcept {
        return {
                lhs.x * rhs.m[0][0] + lhs.y * rhs.m[1][0] + lhs.z * rhs.m[2][0] + rhs.m[3][0],
                lhs.x * rhs.m[0][1] + lhs.y * rhs.m[1][1] + lhs.z * rhs.m[2][1] + rhs.m[3][1],
                lhs.x * rhs.m[0][2] + lhs.y * rhs.m[1][2] + lhs.z * rhs.m[2][2] + rhs.m[3][2] };
    }

    //俺が追加
    friend inline constexpr Vector3 operator*(const Matrix4x4& lhs, const Vector3& rhs) noexcept {
        Vector3 result;

        result.x = lhs.m[0][0] * rhs.x + lhs.m[0][1] * rhs.y + lhs.m[0][2] * rhs.z + lhs.m[0][3];
        result.y = lhs.m[1][0] * rhs.x + lhs.m[1][1] * rhs.y + lhs.m[1][2] * rhs.z + lhs.m[1][3];
        result.z = lhs.m[2][0] * rhs.x + lhs.m[2][1] * rhs.y + lhs.m[2][2] * rhs.z + lhs.m[2][3];

        float w = lhs.m[3][0] * rhs.x + lhs.m[3][1] * rhs.y + lhs.m[3][2] * rhs.z + lhs.m[3][3];

        if (w != 0.0f) {
            result.x /= w;
            result.y /= w;
            result.z /= w;
        }

        return result;
    }

    friend inline constexpr Vector4 operator*(const Vector4& lhs, const Matrix4x4& rhs) noexcept {
        return {
            lhs.x * rhs.m[0][0] + lhs.y * rhs.m[1][0] + lhs.z * rhs.m[2][0] + lhs.w * rhs.m[3][0],
            lhs.x * rhs.m[0][1] + lhs.y * rhs.m[1][1] + lhs.z * rhs.m[2][1] + lhs.w * rhs.m[3][1],
            lhs.x * rhs.m[0][2] + lhs.y * rhs.m[1][2] + lhs.z * rhs.m[2][2] + lhs.w * rhs.m[3][2],
            lhs.x * rhs.m[0][3] + lhs.y * rhs.m[1][3] + lhs.z * rhs.m[2][3] + lhs.w * rhs.m[3][3] };
    }

    //俺が追加
    friend inline constexpr Vector4 operator*(const Matrix4x4& lhs, const Vector4& rhs) {
        Vector4 result;
        result.x = lhs.m[0][0] * rhs.x + lhs.m[0][1] * rhs.y + lhs.m[0][2] * rhs.z + lhs.m[0][3] * rhs.w;
        result.y = lhs.m[1][0] * rhs.x + lhs.m[1][1] * rhs.y + lhs.m[1][2] * rhs.z + lhs.m[1][3] * rhs.w;
        result.z = lhs.m[2][0] * rhs.x + lhs.m[2][1] * rhs.y + lhs.m[2][2] * rhs.z + lhs.m[2][3] * rhs.w;
        result.w = lhs.m[3][0] * rhs.x + lhs.m[3][1] * rhs.y + lhs.m[3][2] * rhs.z + lhs.m[3][3] * rhs.w;
        return result;
    }
    friend inline constexpr Matrix4x4 operator*(float lhs, const Matrix4x4& rhs) noexcept {
        return {
            lhs * rhs.m[0][0], lhs * rhs.m[0][1], lhs * rhs.m[0][2], lhs * rhs.m[0][3],
            lhs * rhs.m[1][0], lhs * rhs.m[1][1], lhs * rhs.m[1][2], lhs * rhs.m[1][3],
            lhs * rhs.m[2][0], lhs * rhs.m[2][1], lhs * rhs.m[2][2], lhs * rhs.m[2][3],
            lhs * rhs.m[3][0], lhs * rhs.m[3][1], lhs * rhs.m[3][2], lhs * rhs.m[3][3] };
    }
    friend inline constexpr Matrix4x4 operator*(const Matrix4x4& lhs, float rhs) noexcept {
        return {
            lhs.m[0][0] * rhs, lhs.m[0][1] * rhs, lhs.m[0][2] * rhs, lhs.m[0][3] * rhs,
            lhs.m[1][0] * rhs, lhs.m[1][1] * rhs, lhs.m[1][2] * rhs, lhs.m[1][3] * rhs,
            lhs.m[2][0] * rhs, lhs.m[2][1] * rhs, lhs.m[2][2] * rhs, lhs.m[2][3] * rhs,
            lhs.m[3][0] * rhs, lhs.m[3][1] * rhs, lhs.m[3][2] * rhs, lhs.m[3][3] * rhs };
    }
    friend inline Matrix4x4& operator*=(Matrix4x4& lhs, const Matrix4x4& rhs) noexcept {
        lhs = lhs * rhs;
        return lhs;
    }
#pragma endregion
#pragma region メンバ関数
    // ベクトルに回転を適用
    inline constexpr Vector3 ApplyRotation(const Vector3& vector) const noexcept {
        return {
            vector.x * m[0][0] + vector.y * m[1][0] + vector.z * m[2][0],
            vector.x * m[0][1] + vector.y * m[1][1] + vector.z * m[2][1],
            vector.x * m[0][2] + vector.y * m[1][2] + vector.z * m[2][2] };
    }
    // ベクトルに変換を適用しWで割る
    inline constexpr Vector3 ApplyTransformWDivide(const Vector3& vector) const noexcept {
        Vector3 result{
            vector.x * m[0][0] + vector.y * m[1][0] + vector.z * m[2][0] + m[3][0],
            vector.x * m[0][1] + vector.y * m[1][1] + vector.z * m[2][1] + m[3][1],
            vector.x * m[0][2] + vector.y * m[1][2] + vector.z * m[2][2] + m[3][2] };
        float w = vector.x * m[0][3] + vector.y * m[1][3] + vector.z * m[2][3] + m[3][3];
        result /= w;
        return result;
    }
    inline void GetAffineValue(Vector3& scale, Quaternion& rotate, Vector3& translate) const {
        scale = GetScale();
        rotate = GetRotate();
        translate = GetTranslate();
    }
    // 行
    inline constexpr Matrix4x4& SetRow(size_t i, const Vector4& v) noexcept {
        m[i][0] = v.x, m[i][1] = v.y, m[i][2] = v.z, m[i][3] = v.w;
        return *this;
    }
    // 行
    inline constexpr Vector4 GetRow(size_t i) const noexcept {
        return { m[i][0], m[i][1], m[i][2], m[i][3] };
    }
    // 列
    inline constexpr Matrix4x4& SetColumn(size_t i, const Vector4& v) noexcept {
        m[0][i] = v.x, m[1][i] = v.y, m[2][i] = v.z, m[3][i] = v.w;
        return *this;
    }
    // 列
    inline constexpr Vector4 GetColumn(size_t i) const noexcept {
        return { m[0][i], m[1][i], m[2][i], m[3][i] };
    }
    inline constexpr Matrix4x4& SetXAxis(const Vector3& v) noexcept {
        m[0][0] = v.x, m[0][1] = v.y, m[0][2] = v.z;
        return *this;
    }
    inline constexpr Vector3 GetXAxis() const noexcept {
        return { m[0][0], m[0][1], m[0][2] };
    }
    inline constexpr Matrix4x4& SetYAxis(const Vector3& v) noexcept {
        m[1][0] = v.x, m[1][1] = v.y, m[1][2] = v.z;
        return *this;
    }
    inline constexpr Vector3 GetYAxis() const noexcept {
        return { m[1][0], m[1][1], m[1][2] };
    }
    inline constexpr Matrix4x4& SetZAxis(const Vector3& v) noexcept {
        m[2][0] = v.x, m[2][1] = v.y, m[2][2] = v.z;
        return *this;
    }
    inline constexpr Vector3 GetZAxis() const noexcept {
        return { m[2][0], m[2][1], m[2][2] };
    }
    inline Vector3 GetScale() const noexcept {
        return { GetXAxis().Length(), GetYAxis().Length(), GetZAxis().Length() };
    }
    inline Quaternion GetRotate() const noexcept {
        return Quaternion::MakeFromOrthonormal(GetXAxis().Normalize(), GetYAxis().Normalize(), GetZAxis().Normalize());
    }
    inline constexpr Matrix4x4& SetTranslate(const Vector3& v) noexcept {
        m[3][0] = v.x, m[3][1] = v.y, m[3][2] = v.z;
        return *this;
    }
    inline constexpr Vector3 GetTranslate() const noexcept {
        return { m[3][0], m[3][1], m[3][2] };
    }
    float Determinant() const noexcept;
    Matrix4x4 Adjugate() const noexcept;
    inline Matrix4x4 Inverse() const noexcept {
        //return 1.0f / Determinant() * Adjugate();
        
        DirectX::XMMATRIX matrix = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(m));
        DirectX::XMVECTOR determinant;

        DirectX::XMMATRIX inverse = DirectX::XMMatrixInverse(&determinant, matrix);
      
        Matrix4x4 result;
        DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(result.m), inverse);
        return result;
    }
    inline constexpr Matrix4x4 Transpose() const noexcept {
        return {
            m[0][0], m[1][0], m[2][0], m[3][0],
            m[0][1], m[1][1], m[2][1], m[3][1],
            m[0][2], m[1][2], m[2][2], m[3][2],
            m[0][3], m[1][3], m[2][3], m[3][3] };
    }
#pragma endregion
#pragma region メンバ変数
    float m[4][4];
#pragma endregion
};


