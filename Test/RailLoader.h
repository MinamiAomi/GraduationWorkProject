#pragma once

#include <optional>
#include <filesystem>

#include "Externals/nlohmann/json.hpp"

#include "RailData.h"
#include "AnimationUtils.h"

namespace RailSystem {
	class AnimationLoader {
	public:
		static std::optional<RailAnimation> LoadAnimation(const std::filesystem::path& filepath);
	private:
		static AnimationUtils::ScalarKeyframe ParseScalarKeyframe(const nlohmann::json& key);
		static AnimationUtils::PositionKeyframe ParsePositionKeyframe(const nlohmann::json& key);
		static AnimationUtils::RotationKeyframe ParseRotationKeyframe(const nlohmann::json& key);
	};
}