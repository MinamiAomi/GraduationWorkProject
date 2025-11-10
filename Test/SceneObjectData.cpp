#include "SceneObjectData.h"
#include "File/JsonConverter.h"

namespace SceneObjectSystem {

	void from_json(const nlohmann::json& j, ObbCollision& o) {
		j.at("world_center").get_to(o.center);
		j.at("world_rotation_quat").get_to(o.rotation);
		j.at("local_size").get_to(o.size);
	}

	void from_json(const nlohmann::json& j, SceneObject& s) {
		j.at("name").get_to(s.name);
		j.at("model_name").get_to(s.modelName);
		j.at("srt").get_to(s.transform);
		j.at("obb_collision").get_to(s.obbCollision);
		j.at("is_emissive").get_to(s.isEmissive);
	}

}