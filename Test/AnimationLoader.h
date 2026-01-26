#pragma once
#include <optional>
#include <filesystem>

#include "Externals/nlohmann/json.hpp"


#include "RailData.h"

#include "SceneObjectData.h"

namespace AnimationUtils {
	class AnimationLoader {
	public:
		static std::optional<RailSystem::RailAnimation> LoadRailAnimation(const std::filesystem::path& filepath);
		static std::optional<std::tuple<std::vector<ScalarKeyframe>, std::vector<PositionKeyframe>, std::vector<RotationKeyframe > >> LoadAnimation(const nlohmann::json& data);
	private:
		static ScalarKeyframe ParseScalarKeyframe(const nlohmann::json& key);
		static PositionKeyframe ParsePositionKeyframe(const nlohmann::json& key);
		static RotationKeyframe ParseRotationKeyframe(const nlohmann::json& key);
	};


	void from_json(const nlohmann::json& j, AnimationUtils::ScalarKeyframe& k);
	void from_json(const nlohmann::json& j, AnimationUtils::PositionKeyframe& k);
	void from_json(const nlohmann::json& j, AnimationUtils::RotationKeyframe& k);
}