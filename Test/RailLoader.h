#pragma once

#include <optional>
#include <filesystem>

#include "RailData.h"

#include "Externals/nlohmann/json.hpp"

namespace RailSystem {
	class AnimationLoader {
	public:
		static std::optional<RailAnimation> LoadAnimation(const std::filesystem::path& filepath);
	private:
		static ScalarKeyframe ParseScalarKeyframe(const nlohmann::json& key);
		static PositionKeyframe ParsePositionKeyframe(const nlohmann::json& key);
		static RotationKeyframe ParseRotationKeyframe(const nlohmann::json& key);
	};
}