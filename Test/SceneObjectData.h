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

namespace SceneObjectSystem {
	enum class ObjectType {
		PointLight,
		EnemySpawn,
		Gimmick,
		Unknown
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(ObjectType, {
		{ObjectType::Unknown, nullptr},
		{ObjectType::PointLight, "POINTLIGHT"},
		{ObjectType::EnemySpawn, "ENEMY_SPAWN"},
		{ObjectType::Gimmick, "GIMMICK"},
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
	};

	struct GimmickTrigger {
		std::string key;

		bool hasTriggered = false;
		bool isOnce;

		std::shared_ptr<Collider> collider;
	};

	struct GimmickMover {
		std::string modelName;
		std::string key;

		float duration;
		bool isCyclic;
		bool isActive = false;
		std::vector<Vector3> positionKeys;
		Transform transform;
		void Update();
	};

	struct PointLightObject {
		ModelInstance model;
		std::shared_ptr<Material> material;
		Transform transform;
		std::shared_ptr<Collider> collider;
		LightObject lightObject;
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
		std::optional<GimmickTrigger> gimmickTriggers;
		std::optional<GimmickMover> gimmickMovers;
	};

	void from_json(const nlohmann::json& j, CapsuleCollisionData& o);
	void from_json(const nlohmann::json& j, SphereCollisionData& o);
	void from_json(const nlohmann::json& j, SceneObjectData& s);
	void from_json(const nlohmann::json& j, PointLightData& p);
	void from_json(const nlohmann::json& j, EnemySpawnData& p);
	void from_json(const nlohmann::json& j, GimmickTrigger& p);
	void from_json(const nlohmann::json& j, GimmickMover& p);
}