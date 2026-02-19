#include "AnimationTransform.h"

void AnimationTransform::Initialize(const std::shared_ptr<AnimationAsset>& asset, const std::string& animationN, const std::string& nodeN) {
    animation = asset;
    animationName = animationN;
    nodeName = nodeN;
    animationTime = 0.0f;
}

void AnimationTransform::Update(float deltaTime, bool isLoop) {
    if (!animation) {
        return;
    }
    const AnimationSet& anime = animation->Get()->GetAnimation(animationName);
    auto it = anime.nodeAnimations.find(nodeName);
    if (animationTime <= 1.0f) {
        animationTime += deltaTime / anime.duration;
    }
    else {
        animationTime = isLoop ? animationTime - 1.0f : 1.0f;
    }
    if (it != anime.nodeAnimations.end()) {
        const NodeAnimation& nodeAnim = it->second;
        transform.translate = CalculateValue(nodeAnim.translate, animationTime);
        transform.rotate = CalculateValue(nodeAnim.rotate, animationTime);
        transform.scale = CalculateValue(nodeAnim.scale, animationTime);
    }
    transform.UpdateMatrix();
}
