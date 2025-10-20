#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Node.h"

// キーフレーム
template<typename T>
struct Keyframe {
    float time;
    T value;
};
// アニメーションカーブ
template<typename T>
struct AnimationCurve {
    std::vector<Keyframe<T>> keyframes;
};
Vector3 CalculateValue(const AnimationCurve<Vector3>& animationCurve, float time);
Quaternion CalculateValue(const AnimationCurve<Quaternion>& animationCurve, float time);

// ノードアニメーション
struct NodeAnimation {
    AnimationCurve<Vector3> translate;
    AnimationCurve<Quaternion> rotate;
    AnimationCurve<Vector3> scale;
};
// アニメーション
struct AnimationSet {
    float duration;
    std::map<std::string, NodeAnimation> nodeAnimations;
};

class Animation {
public:
    static std::shared_ptr<Animation> Load(const std::filesystem::path& path);

    AnimationSet& GetAnimation(const std::string& name) { return animationSet_.at(name); }

private:
    std::map<std::string, AnimationSet> animationSet_;
};
