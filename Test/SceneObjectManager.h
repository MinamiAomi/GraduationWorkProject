#pragma once

#include <memory>

#include "SceneObjectData.h"
#include "Framework/AssetManager.h"

namespace SceneObjectSystem {
	class SceneObjectManager {
	public:
		void Initialize();
		//最初の一回ロード用
		void CreateObjects(const std::vector<SceneObjectSystem::SceneObjectData>& objectData);
		//ロードしたデータを基に再設置
		void ResetObjects();
		void Update();

		//ゲームで使う方	
		const std::vector<std::unique_ptr<SceneObjectSystem::PointLightObject>>& GetPointLightObjects()const { return pointLightObjects_; }
		const std::vector<std::unique_ptr<SceneObjectSystem::EmitterObject>>& GetEmitterObjects()const { return emitterObjects_; }
		const std::vector<std::unique_ptr<SceneObjectSystem::EnemyObject>>& GetEnemyObjects()const { return enemyObjects_; }
		//Blenderの値そのままデータだけ
		const std::vector<std::unique_ptr<SceneObjectSystem::SceneObjectData>>& GetSceneObjectData()const { return sceneObjectData_; }
	private:
		// 共通の初期化処理を行うテンプレート関数
		template <typename T>
		void InitializeCommonObject(T& targetObj, const SceneObjectSystem::SceneObjectData& sourceData);

		// 実際にRuntimeオブジェクトリストを構築する関数
		void BuildRuntimeObjects();
		//ステージ全体
		ModelInstance stageObjects_;
		//ゲームで使う方
		std::vector<std::unique_ptr<SceneObjectSystem::PointLightObject>> pointLightObjects_;
		std::vector<std::unique_ptr<SceneObjectSystem::EmitterObject>> emitterObjects_;
		std::vector<std::unique_ptr<SceneObjectSystem::EnemyObject>> enemyObjects_;

		//Blenderの値そのままデータだけ
		std::vector<std::unique_ptr<SceneObjectSystem::SceneObjectData>> sceneObjectData_;
	};
	template<typename T>
	inline void SceneObjectManager::InitializeCommonObject(T& targetObj, const SceneObjectSystem::SceneObjectData& sourceData)
	{
		const auto& assetManager = AssetManager::GetInstance();

		if (auto modelHandle = assetManager->modelMap.Get(sourceData.modelName)) {
			targetObj->model.SetModel(modelHandle->Get());
		}

		targetObj->transform = sourceData.transform;
		targetObj->transform.UpdateMatrix();

		if (sourceData.capsuleCollisionData) {
			targetObj->collider = std::make_shared<CapsuleCollider>(
				CollisionCategory::LIGHT,
				CollisionCategory::FLASHLIGHT,
				Vector3::zero,
				0.0f,
				0.0f,
				Quaternion::identity
			);

			const auto& colData = sourceData.capsuleCollisionData.value();
			targetObj->collider->center = colData.center;
			targetObj->collider->quaternion = colData.quaternion;
			targetObj->collider->radius = colData.radius;
			targetObj->collider->height = colData.height;
		}
	}
}