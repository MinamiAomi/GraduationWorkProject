#pragma once

#include <fstream>

#include "SceneObjectData.h"

namespace SceneObjectSystem {

	class SceneLoader {
	public:
		static std::vector<SceneObjectSystem::SceneObject> LoadSceneFromFile(const std::filesystem::path& filePath);
	};
}