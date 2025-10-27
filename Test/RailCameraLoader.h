#pragma once

#include <optional>
#include <filesystem>

#include "RailCameraData.h"

#include "Externals/nlohmann/json.hpp"

namespace RailCameraSystem {
	class AnimationLoader {
	public:
		static std::optional<RailCameraAnimation> LoadAnimation(const std::filesystem::path& filepath);
	private:
		static ScalarKeyframe ParseScalarKeyframe(const nlohmann::json& key);
		static PositionKeyframe ParsePositionKeyframe(const nlohmann::json& key);
		static RotationKeyframe ParseRotationKeyframe(const nlohmann::json& key);
	};
}