#include "SceneObjectData.h"
#include "File/JsonConverter.h"

namespace SceneObjectSystem {

	void from_json(const nlohmann::json& j, CapsuleCollisionData& o) {
		j.at("center").get_to(o.center);
		j.at("radius").get_to(o.radius);
		j.at("height").get_to(o.height);
		j.at("quaternion").get_to(o.quaternion);
	}

	void from_json(const nlohmann::json& j, SceneObjectData& s) {
		j.at("name").get_to(s.name);
		j.at("model_name").get_to(s.modelName);
		j.at("srt").get_to(s.transform);

		//なかった場合nullopt
		if (j.contains("capsule_collision") && !j.at("capsule_collision").is_null()) {
			s.capsuleCollisionData = j.at("capsule_collision").get<CapsuleCollisionData>();
		}
		else {
			s.capsuleCollisionData = std::nullopt;
		}
		j.at("is_emissive").get_to(s.isEmissive);
	}

}