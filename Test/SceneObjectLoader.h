#pragma once

#include <fstream>

#include "SceneObjectData.h"

namespace SceneObjectSystem {

	class SceneLoader {
	public:
		static std::vector<SceneObjectSystem::SceneObjectData> LoadSceneFromFile(const std::filesystem::path& filePath);
	};
}