#include "Geometry.h"

namespace Math {

    bool IsCollision(const Sphere& sphere1, const Sphere& sphere2) {
        return (sphere1.center - sphere2.center).LengthSquare() <= (sphere1.radius + sphere2.radius) * (sphere1.radius + sphere2.radius);
    }

    bool IsCollision(const Sphere& sphere, const AABB& aabb) {
        Vector3 point = {
            std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
            std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
            std::clamp(sphere.center.z, aabb.min.z, aabb.max.z) };
        return (point - sphere.center).LengthSquare() <= sphere.radius * sphere.radius;
    }

 /*   bool IsCollision(const Sphere& sphere, const OBB& obb) {
        Vector3 centerInOBBLocal = obb.orientation.Inverse() * (sphere.center - obb.center);
        AABB aabbInOBBLocal{ -obb.size, obb.size };
        Sphere sphereInOBBLocal{ centerInOBBLocal, sphere.radius };
        return IsCollision(sphereInOBBLocal, aabbInOBBLocal);
    }*/

    bool IsCollision(const OBB& obb1, const OBB& obb2) {
        obb1, obb2;
        return false;
    }

}