#pragma once

#include <optional>
#include <filesystem>

#include "RailCameraData.h"

namespace RailCameraSystem {
	class AnimationLoader {
	public:
		static std::optional<RailCameraAnimation> LoadAnimation(const std::filesystem::path& filepath);
	};
}