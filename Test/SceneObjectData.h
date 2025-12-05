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

	struct CapsuleCollisionData {
		Vector3 center;
		float radius;
		float height;
		Quaternion quaternion;
	};

	struct SceneObjectData {
		std::string name;
		std::string modelName;
		Transform transform;
		std::optional<CapsuleCollisionData> capsuleCollisionData;
		bool isEmissive;
	};

	struct SceneObject {
		ModelInstance model;
		Transform transform;
		std::optional<std::shared_ptr<CapsuleCollider>> collider;
		LightObject lightObject;
		bool isEmissive;
	};

	void from_json(const nlohmann::json& j, CapsuleCollisionData& o);
	void from_json(const nlohmann::json& j, SceneObjectData& s);
}