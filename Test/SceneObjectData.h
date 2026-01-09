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
		Enemy,
		Emitter,
		Unknown
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(ObjectType, {
		{ObjectType::Unknown, nullptr},
		{ObjectType::PointLight, "POINTLIGHT"},
		{ObjectType::Enemy, "ENEMY"},
		{ObjectType::Emitter, "EMITTER"},
		})

	struct CapsuleCollisionData {
		Vector3 center;
		float radius;
		float height;
		Quaternion quaternion;
	};

	struct PointLightData {
		Vector3 offset;
		Color color;
		float intensity;
		float range;
		float decay;
		bool isActive;
	};

	struct EmitterData {
		std::string name;
	};

	struct EnemyData {
		std::string name;
	};

	struct PointLightObject {
		ModelInstance model;
		std::shared_ptr<Material> material;
		Transform transform;
		std::shared_ptr<CapsuleCollider> collider;
		LightObject lightObject;
	};

	struct EmitterObject {
		ModelInstance model;
		Transform transform;
		std::shared_ptr<CapsuleCollider> collider;
		EmitterData emitter;
	};

	struct EnemyObject {
		ModelInstance model;
		Transform transform;
		std::shared_ptr<CapsuleCollider> collider;
		EnemyData enemy;
	};

	struct SceneObjectData {
		std::string name;
		std::string modelName;
		ObjectType type;
		Transform transform;
		std::optional<CapsuleCollisionData> capsuleCollisionData;
		std::optional<PointLightData> pointLightData;
		std::optional<EmitterData> emitterData;
		std::optional<EnemyData> enemyData;
	};

	void from_json(const nlohmann::json& j, CapsuleCollisionData& o);
	void from_json(const nlohmann::json& j, SceneObjectData& s);
	void from_json(const nlohmann::json& j, PointLightData& p);
	void from_json(const nlohmann::json& j, EmitterData& p);
	void from_json(const nlohmann::json& j, EnemyData& p);
}