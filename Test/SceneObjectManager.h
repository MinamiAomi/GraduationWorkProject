#pragma once

#include <memory>

#include "Collider.h"
#include "SceneObjectData.h"
#include "Framework/AssetManager.h"

#include "Math/Random.h"

#include "BatsManager.h"
#include "GhostsManager.h"

namespace SceneObjectSystem {
	class SceneObjectManager {
	public:
		void Initialize();
		//最初の一回ロード用
		void CreateObjects(const std::vector<SceneObjectSystem::SceneObjectData>& objectData, const std::string& stageName);
		//ロードしたデータを基に再設置
		void ResetObjects();
		void Update();

		//ゲームで使う方	
		const std::vector<std::unique_ptr<SceneObjectSystem::PointLightObject>>& GetPointLightObjects()const { return pointLightObjects_; }
		const std::vector<std::unique_ptr<SceneObjectSystem::EnemySpawnObject>>& GetEnemySpawnObjects()const { return enemySpawnObjects_; }
		const std::vector<std::unique_ptr<SceneObjectSystem::GimmickMoverObject>>& GetGimmickMoverObjects()const { return gimmickMoverObjects_; }
		const std::vector<std::unique_ptr<SceneObjectSystem::GimmickTriggerObject>>& GetGimmickTriggerObjects()const { return gimmickTriggerObjects_; }
		const std::vector<std::unique_ptr<SceneObjectSystem::GimmickPointLightObject>>& GetGimmickPointLightObjects()const { return gimmickPointLightObjects_; }
		const std::vector<std::unique_ptr<SceneObjectSystem::ObstacleObject>>& GetObstacleObjects()const { return obstacleObjects_; }
		//Blenderの値そのままデータだけ
		const std::vector<std::unique_ptr<SceneObjectSystem::SceneObjectData>>& GetSceneObjectData()const { return sceneObjectData_; }

		void SetBatsManager(BatsManager* batsManager) { batsManager_ = batsManager; }
		void SetGhostsManager(GhostsManager* ghostsManager) { ghostsManager_ = ghostsManager; }
	private:
		Random::RandomNumberGenerator rnd_;

		// 共通の初期化処理を行うテンプレート関数
		template <typename T>
		void InitializeCommonObject(
			T& targetObj, 
			const SceneObjectSystem::SceneObjectData& sourceData,
			uint32_t myCategory,
			uint32_t targetMask
		);

		// 実際にRuntimeオブジェクトリストを構築する関数
		void BuildRuntimeObjects();

		//ステージ全体
		ModelInstance stageObjects_;
		//ゲームで使う方
		std::vector<std::unique_ptr<SceneObjectSystem::PointLightObject>> pointLightObjects_;
		std::vector<std::unique_ptr<SceneObjectSystem::EnemySpawnObject>> enemySpawnObjects_;
		std::vector<std::unique_ptr<SceneObjectSystem::GimmickMoverObject>> gimmickMoverObjects_;
		std::vector<std::unique_ptr<SceneObjectSystem::GimmickTriggerObject>> gimmickTriggerObjects_;
		std::vector<std::unique_ptr<SceneObjectSystem::GimmickPointLightObject>> gimmickPointLightObjects_;
		std::vector<std::unique_ptr<SceneObjectSystem::ObstacleObject>> obstacleObjects_;

		//Blenderの値そのままデータだけ
		std::vector<std::unique_ptr<SceneObjectSystem::SceneObjectData>> sceneObjectData_;

		struct PointLightParameters {
			// スケール(1,1,1)の時の基準HP
			float baseHp = 10.0f;

			// サイズが大きくなった時のHP増加率（0.0ならサイズ無視、1.0ならサイズ通りにHP増）
			float sizeCorrectionFactor = 1.0f;

			// プレイヤーから受けるダメージ量
			float damageReceived = 1.0f;
		};

		struct SceneObjectConfig {
			void Initialize();
#ifdef _DEBUG
			void DrawImGui();
#endif // _DEBUG
			PointLightParameters pointLightParams;

		} sceneObjectConfig_;

		BatsManager* batsManager_;
		GhostsManager* ghostsManager_;
	};

	template<typename T>
	inline void SceneObjectManager::InitializeCommonObject(
		T& targetObj, 
		const SceneObjectSystem::SceneObjectData& sourceData,
		uint32_t myCategory,
		uint32_t targetMask
	)
	{
		if constexpr (requires { targetObj->transform; }) {
			if constexpr (requires {sourceData.transform; }) {
				targetObj->transform = sourceData.transform;
				targetObj->transform.UpdateMatrix();
			}
		}

		if constexpr (requires { targetObj->collider; }) {


			if (sourceData.sphereCollisionData) {
				auto sphereCol = std::make_shared<SphereCollider>(
					CollisionCategory(myCategory),
					CollisionCategory(targetMask),
					Vector3::zero,
					0.0f
				);

				const auto& colData = sourceData.sphereCollisionData.value();
				sphereCol->center = colData.center;
				sphereCol->radius = colData.radius;

				targetObj->collider = sphereCol;
			}

			if (sourceData.capsuleCollisionData) {
				auto capsuleCol = std::make_shared<CapsuleCollider>(
					CollisionCategory(myCategory),
					CollisionCategory(targetMask),
					Vector3::zero,
					0.0f,
					0.0f,
					Quaternion::identity
				);

				const auto& colData = sourceData.capsuleCollisionData.value();
				capsuleCol->center = colData.center;
				capsuleCol->quaternion = colData.quaternion;
				capsuleCol->radius = colData.radius;
				capsuleCol->height = colData.height;

				targetObj->collider = capsuleCol;
			}
		}
	}
}