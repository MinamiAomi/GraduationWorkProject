#pragma once

#include <string>
#include <vector>

#include "Externals/nlohmann/json.hpp"

#include "Graphics/Model.h"

#include "Math/Transform.h"
#include "Math/MathUtils.h"

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
		ObbCollision obbCollision;
		bool isEmissive;
	};
	struct SceneObject{
		ModelInstance model_;
		Transform transform;
		ObbCollision obbCollision;
		bool isEmissive;
	};

	void from_json(const nlohmann::json& j, ObbCollision& o);
	void from_json(const nlohmann::json& j, SceneObjectData& s);
}