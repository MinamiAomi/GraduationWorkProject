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
		EventTrigger,
		Unknown
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(ObjectType, {
		{ObjectType::Unknown, nullptr},
		{ObjectType::PointLight, "POINTLIGHT"},
		{ObjectType::EventTrigger, "EVENT_TRIGGER"},
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

	struct EnemyTriggerData {
		bool isOnce;

		std::vector<std::vector<bool>> formation;
	};

	struct EventTriggerData {
		bool isOnce;
		float distance;
	};

	struct EventTriggerTypeObject {
		Transform transform;

		//既に実行したか
		bool hasTriggered = false;

		std::shared_ptr<Collider> collider;
		std::optional<EnemyTriggerData> enemyTrigger;
		std::optional<EventTriggerData> eventTrigger;
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
		std::optional<EventTriggerTypeObject> eventTriggerTypeData;
	};

	void from_json(const nlohmann::json& j, CapsuleCollisionData& o);
	void from_json(const nlohmann::json& j, SphereCollisionData& o);
	void from_json(const nlohmann::json& j, SceneObjectData& s);
	void from_json(const nlohmann::json& j, PointLightData& p);
	void from_json(const nlohmann::json& j, EventTriggerTypeObject& p);
	void from_json(const nlohmann::json& j, EnemyTriggerData& p);
	void from_json(const nlohmann::json& j, EventTriggerData& p);
}