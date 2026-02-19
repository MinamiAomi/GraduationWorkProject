#pragma once

#include <memory>

#include "Graphics/Model.h"
#include "Math/Transform.h"
#include "Framework/AssetManager.h"

class AnimationTransform {
public:
    void Initialize(const std::shared_ptr<AnimationAsset>& asset, const std::string& animationName, const std::string& nodeName);
	void Update(float deltaTime, bool isLoop = false);

	Transform transform;
	std::shared_ptr<AnimationAsset> animation;
    std::string animationName;
    std::string nodeName;
	float animationTime;
};