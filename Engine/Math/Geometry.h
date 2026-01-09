#pragma once

#include <vector>
#include <array>

#include "MathUtils.h"

namespace Math {

    struct Sphere;
    struct AABB;
    struct OBB;
    struct Line;
    struct Ray;
    struct Segment;
    struct Capsule;
    struct Plane;
    struct Triangle;


    struct Sphere {
        Vector3 center;
        float radius;
    };

    struct AABB {
        AABB() = default;
        AABB(const Vector3& min, const Vector3& max) : min(min), max(max) {}
        explicit AABB(const Vector3& minmax) : min(minmax), max(minmax) {}
        explicit AABB(const std::vector<Vector3>& points) : min(points[0]), max(points[0]) {
            for (size_t i = 1; i < points.size(); ++i) {
                Merge(points[i]);
            }
        }

        /// <summary>
        /// 二つを融合する
        /// </summary>
        /// <param name="other"></param>
        void Merge(const AABB& other) {
            min = Vector3::Min(min, other.min);
            max = Vector3::Max(max, other.max);
        }
        /// <summary>
        /// 点を含める
        /// </summary>
        /// <param name="point"></param>
        void Merge(const Vector3& point) {
            min = Vector3::Min(min, point);
            max = Vector3::Max(max, point);
        }

        /// <summary>
        /// 幅
        /// </summary>
        /// <returns></returns>
        Vector3 Extent() const { return max - min; }
        /// <summary>
        /// 幅
        /// </summary>
        /// <param name="dim"></param>
        /// <returns></returns>
        float Extent(size_t dim) const { return max[dim] - min[dim]; }
        /// <summary>
        /// 中央
        /// </summary>
        /// <returns></returns>
        Vector3 Center() const { return (max + min) * 0.5f; }
        /// <summary>
        /// 中央
        /// </summary>
        /// <param name="dim"></param>
        /// <returns></returns>
        float Center(size_t dim) const { return (max[dim] + min[dim]) * 0.5f; }

        /// <summary>
        /// 含むか
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        bool Contains(const AABB& other) const {
            return
                min.x <= other.min.x &&
                other.max.x <= max.x &&
                min.y <= other.min.y &&
                other.max.y <= max.y &&
                min.z <= other.min.z &&
                other.max.z <= max.z;
        }
        /// <summary>
        /// 含むか
        /// </summary>
        /// <param name="point"></param>
        /// <returns></returns>
        bool Contains(const Vector3& point) const {
            return
                min.x <= point.x &&
                point.x <= max.x &&
                min.y <= point.y &&
                point.y <= max.y &&
                min.z <= point.z &&
                point.z <= max.z;
        }

        Vector3 min;
        Vector3 max;
    };

    struct OBB {
        Vector3 center;
        Vector3 orientations[3];
        Vector3 size;
    };

    struct Line {
        Vector3 origin;
        Vector3 diff;
    };

    struct Ray {
        Vector3 origin;
        Vector3 diff;
    };

    struct Segment {
        Vector3 origin;
        Vector3 diff;
    };

    struct Capsule {
        Segment segment;
        float radius;
    };

    struct Triangle {
        Triangle() = default;
        Triangle(const Vector3& v0, const Vector3& v1, const Vector3& v2) {
            vertices[0] = v0;
            vertices[1] = v1;
            vertices[2] = v2;
        }
        /// <summary>
        /// 法線を取得
        /// </summary>
        /// <returns></returns>
        Vector3 Normal() const {
            return Cross(vertices[1] - vertices[0], vertices[2] - vertices[1]).Normalized();
        }

        Vector3 vertices[3];
    };

    struct Plane {
        Plane() = default;
        Plane(const Vector3& normal, float distance) :
            normal(normal),
            distance(distance) {
        }
        Plane(const Vector3& normal, const Vector3& point) :
            normal(normal.Normalized()),
            distance(0.0f) {
            distance = Dot(normal, point);
        }
        explicit Plane(const Triangle& triangle) :
            Plane(triangle.Normal(), triangle.vertices[0]) {
        }


        Vector3 normal;
        float distance;
    };

    class Frustum {
    public:
        enum Side {
            Near = 0,
            Far,
            Left,
            Right,
            Top,
            Bottom,
            Count
        };

        Frustum() = default;

        void Set(const Matrix4x4& vp) {

            planes[Side::Left].normal.x = vp.m[0][3] + vp.m[0][0];
            planes[Side::Left].normal.y = vp.m[1][3] + vp.m[1][0];
            planes[Side::Left].normal.z = vp.m[2][3] + vp.m[2][0];
            planes[Side::Left].distance = -(vp.m[3][3] + vp.m[3][0]);

            planes[Side::Right].normal.x = vp.m[0][3] - vp.m[0][0];
            planes[Side::Right].normal.y = vp.m[1][3] - vp.m[1][0];
            planes[Side::Right].normal.z = vp.m[2][3] - vp.m[2][0];
            planes[Side::Right].distance = -(vp.m[3][3] - vp.m[3][0]);

            planes[Side::Bottom].normal.x = vp.m[0][3] + vp.m[0][1];
            planes[Side::Bottom].normal.y = vp.m[1][3] + vp.m[1][1];
            planes[Side::Bottom].normal.z = vp.m[2][3] + vp.m[2][1];
            planes[Side::Bottom].distance = -(vp.m[3][3] + vp.m[3][1]);

            planes[Side::Top].normal.x = vp.m[0][3] - vp.m[0][1];
            planes[Side::Top].normal.y = vp.m[1][3] - vp.m[1][1];
            planes[Side::Top].normal.z = vp.m[2][3] - vp.m[2][1];
            planes[Side::Top].distance = -(vp.m[3][3] - vp.m[3][1]);

            planes[Side::Near].normal.x = vp.m[0][3] + vp.m[0][2];
            planes[Side::Near].normal.y = vp.m[1][3] + vp.m[1][2];
            planes[Side::Near].normal.z = vp.m[2][3] + vp.m[2][2];
            planes[Side::Near].distance = -(vp.m[3][3] + vp.m[3][2]);

            planes[Side::Far].normal.x = vp.m[0][3] - vp.m[0][2];
            planes[Side::Far].normal.y = vp.m[1][3] - vp.m[1][2];
            planes[Side::Far].normal.z = vp.m[2][3] - vp.m[2][2];
            planes[Side::Far].distance = -(vp.m[3][3] - vp.m[3][2]);

            for (auto& plane : planes) {
                float length = plane.normal.Length();
                if (length > 1e-5f) {
                    float invLength = 1.0f / length;
                    plane.normal *= invLength;
                    plane.distance *= invLength;
                }
            }
        }

        bool Intersects(const Sphere& sphere) const {
            for (const auto& plane : planes) {
                float dist = Dot(plane.normal, sphere.center) - plane.distance;
                if (dist < -sphere.radius) {
                    return false;
                }
            }
            return true;
        }

        bool InersectsSpotLight(const Vector3& position, const Vector3& direction, float range, float angle) const {
            Sphere boundingSphere;
            boundingSphere.center = position;
            boundingSphere.radius = range;

            float offset = range * 0.5f;
            boundingSphere.center = position + direction * offset;
            boundingSphere.radius = range * 0.5f / std::cos(angle);

            return Intersects(boundingSphere);
        }

        bool Contains(const Vector3& point) const {
            for (const auto& plane : planes) {
                float dist = Dot(plane.normal, point) - plane.distance;
                if (dist < 0.0f) {
                    return false;
                }
            }
            return true;
        }

        std::array<Plane, Side::Count> planes;
    };

    bool IsCollision(const Sphere& sphere1, const Sphere& sphere2);
    bool IsCollision(const Sphere& sphere, const AABB& aabb);
    bool IsCollision(const Sphere& sphere, const OBB& obb);
    bool IsCollision(const OBB& obb1, const OBB& obb2);
}