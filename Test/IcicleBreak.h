#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"

#include "ParticleDefine.h"
#include "Engine/Graphics/Animation.h"
#include <Framework/AssetManager.h>

class IcicleBreak
{
public:
	void Initialize();
	void Update();
	void Emit(const Transform& copyTransform);
private:
	struct ModelInfo {
		ModelInstance modelInstance_[4];
		Transform parentTransform_;
		Transform transform_[4];
		std::shared_ptr<AnimationAsset> animation[4];
		float animationTime;
		float duration_;
	};
	std::vector<std::unique_ptr<ModelInfo>> modelInfo_;
};

