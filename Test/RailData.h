#pragma once

#include "AnimationUtils.h"

namespace RailSystem {
	struct RailMetaData {
		int startFrame;
		int endFrame;
		float frameRate;
	};

	struct  NodeAnimation {
		std::vector<AnimationUtils::PositionKeyframe> positionKeys;
		std::vector<AnimationUtils::RotationKeyframe> rotationKeys;
	};
	class RailAnimation {
	public:
		RailMetaData railMetaData_;
		std::vector<AnimationUtils::ScalarKeyframe> evalTimeKeys_;
		NodeAnimation railAnimation_;
		NodeAnimation cameraAnimation_;
	};
}