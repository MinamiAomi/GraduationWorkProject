#pragma once

#include "AnimationUtils.h"

namespace RailSystem {
	struct RailMetaData {
		int startFrame;
		int endFrame;
		float frameRate;
	};

	
	class RailAnimation {
	public:
		RailMetaData railMetaData_;
		std::vector<AnimationUtils::ScalarKeyframe> evalTimeKeys_;
		AnimationUtils::NodeAnimation railAnimation_;
		AnimationUtils::NodeAnimation cameraAnimation_;
	};
}