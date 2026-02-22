#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>

#include "Externals/nlohmann/json.hpp"

#include "Graphics/Model.h"

#include "Math/Transform.h"
#include "Math/MathUtils.h"

#include "Collider.h"

#include "LightObject.h"
#include "PowerEmitter.h"

#include "AnimationUtils.h"
namespace SceneObjectSystem {
	enum class ObjectType {
		PointLight,
		EnemySpawn,
		Gimmick,
		Obstacle,
		Unknown
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(ObjectType, {
		{ObjectType::Unknown, nullptr},
		{ObjectType::PointLight, "POINTLIGHT"},
		{ObjectType::EnemySpawn, "ENEMY_SPAWN"},
		{ObjectType::Gimmick, "GIMMICK"},
		{ObjectType::Obstacle, "OBSTACLE"},
		})


		struct CapsuleCollisionData {
		Vector3 center;
		float radius;
		float height;
		Quaternion quaternion;
	};

	struct SphereCollisionData {
		Vector3 center;
		float radius;
	};

	struct PointLightData {
		Vector3 offset;
		Color color;
		float intensity;
		float range;
		float decay;
		bool isActive;
	};

	struct EnemySpawnData {
		bool hasTriggered = false;
		bool isOnce;
		std::vector<std::vector<bool>> formation;
		std::shared_ptr<Collider> collider;

		std::optional<std::string> name;
		std::optional<Transform> transform;

	};

	struct EnemySpawnObject {
		bool hasTriggered = false;
		bool isOnce;
		std::vector<std::vector<bool>> formation;
		std::shared_ptr<Collider> collider;

		std::optional<ModelInstance> model;
		Transform transform;
	};

	struct GimmickTriggerData {
		std::string key;

		bool isOnce;
	};

	struct GimmickTriggerObject {
		std::string key;

		bool hasTriggered = false;
		bool isOnce;

		std::shared_ptr<Collider> collider;
	};

	struct GimmickMoverData {
		std::string modelName;
		std::string key;

		float duration;
		bool isCyclic;
		std::vector<AnimationUtils::ScalarKeyframe> evalTimeKeys;
		AnimationUtils::NodeAnimation moverAnimation;
	};

	struct GimmickMoverObject {
		Transform transform;
		ModelInstance model;
		std::string key;

		float duration;
		float time;
		bool isCyclic;
		bool isActive = false;
		std::vector<AnimationUtils::ScalarKeyframe> evalTimeKeys;
		AnimationUtils::NodeAnimation moverAnimation;
		void Update();
	};

	struct PointLightObject {
		ModelInstance model;
		std::shared_ptr<Material> material;
		Transform transform;
		std::shared_ptr<Collider> collider;
		LightObject lightObject;
		PowerEmitter powerEmitter_;
		void Update();
	};

	struct GimmickPointLightData {
		PointLightData pointlightData;
		GimmickMoverData gimmickMoverData;
	};

	struct GimmickPointLightObject {
		Transform transform;
		ModelInstance model;
		std::shared_ptr<Material> material;
		std::shared_ptr<Collider> collider;

		struct Pointlight {
			LightObject lightObject;
			PowerEmitter powerEmitter;

			void Update();
		}pointlight;

		struct GimmickMover {
			std::string key;
			float duration;
			float time;
			bool isCyclic;
			bool isActive = false;
			std::vector<AnimationUtils::ScalarKeyframe> evalTimeKeys;
			AnimationUtils::NodeAnimation moverAnimation;

			void Update(Transform& transform);
		}mover;

		void Update();
	};

	struct ObstacleData {
		float hp;
		std::string modelName;
	};

	struct ObstacleObject {
		bool isAlive;
		float hp;
		float maxHp;
		ModelInstance model;
		Transform transform;
		Vector3 basePosition;
		std::shared_ptr<Collider> collider;

		void Update();
		void SetDamage();
	};


	struct SceneObjectData {
		std::optional<std::string> name;
		std::optional<std::string> modelName;
		ObjectType type;
		Transform transform;
		std::optional<CapsuleCollisionData> capsuleCollisionData;
		std::optional<SphereCollisionData> sphereCollisionData;
		std::optional<PointLightData> pointLightData;
		std::optional<EnemySpawnData> enemySpawnData;
		std::optional<GimmickTriggerData> gimmickTriggers;
		std::optional<GimmickMoverData> gimmickMovers;
		std::optional<GimmickPointLightData> gimmickPointlights;
		std::optional<ObstacleData> obstacles;
	};

	void from_json(const nlohmann::json& j, CapsuleCollisionData& o);
	void from_json(const nlohmann::json& j, SphereCollisionData& o);
	void from_json(const nlohmann::json& j, SceneObjectData& s);
	void from_json(const nlohmann::json& j, PointLightData& p);
	void from_json(const nlohmann::json& j, EnemySpawnData& p);
	void from_json(const nlohmann::json& j, GimmickTriggerData& p);
	void from_json(const nlohmann::json& j, GimmickMoverData& p);
	void from_json(const nlohmann::json& j, GimmickPointLightData& p);
	void from_json(const nlohmann::json& j, ObstacleData& p);

}