#pragma once

#include <string>
#include <vector>

#include "Math/MathUtils.h"

struct Node {
    struct Transform {
        Vector3 translate;
        Quaternion rotate;
        Vector3 scale;
    } transform;
    Matrix4x4 localMatrix;
    std::string name;
    std::vector<Node> children;
};

