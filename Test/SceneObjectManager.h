#pragma once

#include <memory>

#include "SceneObjectData.h"

namespace SceneObjectSystem {
	class SceneObjectManager {
	public:
		void Initialize();
		void CreateObjects(const std::vector<SceneObjectSystem::SceneObjectData>& objectData);
		void Update();

		//ゲームで使う方	
		const std::vector<std::unique_ptr<SceneObjectSystem::SceneObject>>& GetSceneObjects()const { return sceneObjects_; }
		//Blenderの値そのままデータだけ
		const std::vector<std::unique_ptr<SceneObjectSystem::SceneObjectData>>& GetSceneObjectData()const { return sceneObjectData_; }
	private:
		//ゲームで使う方
		std::vector<std::unique_ptr<SceneObjectSystem::SceneObject>> sceneObjects_;
		//Blenderの値そのままデータだけ
		std::vector<std::unique_ptr<SceneObjectSystem::SceneObjectData>> sceneObjectData_;
	};
}