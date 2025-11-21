#pragma once

#include <string>
#include <vector>
#include <optional>

#include "Externals/nlohmann/json.hpp"

#include "Graphics/Model.h"

#include "Math/Transform.h"
#include "Math/MathUtils.h"

#include "Collider.h"

namespace SceneObjectSystem {

	struct ObbCollision {
		Vector3 center;
		Quaternion rotation;
		Vector3 size;
	};

	struct SceneObjectData {
		std::string name;
		std::string modelName;
		Transform transform;
		std::optional<ObbCollision> obbCollision;
		bool isEmissive;
	};
	struct SceneObject{
		ModelInstance model_;
		Transform transform;
		std::optional<OBBCollider> obbCollision;
		bool isEmissive;
	};

	void from_json(const nlohmann::json& j, ObbCollision& o);
	void from_json(const nlohmann::json& j, SceneObjectData& s);
}