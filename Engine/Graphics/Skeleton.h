#pragma once
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Animation.h"

class Model;

class Skeleton {
    friend class SkinningManager;
public:
    // ジョイント
    struct Joint {
        struct Transform {
            Vector3 translate;
            Quaternion rotate;
            Vector3 scale;
        } transform;
        Matrix4x4 localMatrix;
        Matrix4x4 skeletonSpaceMatrix;
        std::string name;
        std::vector<int32_t> children;
        int32_t index;
        std::optional<int32_t> parent;
    };

    ~Skeleton();

    void Create(const std::shared_ptr<Model>& model);

    void ApplyAnimation(const AnimationSet& animation, float animationTime);
    void Update();
    void DebugDraw(const Matrix4x4& worldMatrix);

    const Joint& GetRootJoint() const { return joints_.at(root_); }
    const Joint& GetJoint(const std::string& name) const { return joints_.at(jointMap_.at(name)); }
    const Joint& GetJoint(int32_t index) const { return joints_.at(index); }
    const std::vector<Joint>& GetJoints() const { return joints_; }
    const std::map<std::string, int32_t>& GetJointMap() const { return jointMap_; }

private:
    int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);

    int32_t root_;
    std::map<std::string, int32_t> jointMap_;
    std::vector<Joint> joints_;
    bool updated_ = false;
};