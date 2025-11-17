#include "SceneObjectData.h"
#include "File/JsonConverter.h"

namespace SceneObjectSystem {

	void from_json(const nlohmann::json& j, ObbCollision& o) {
		j.at("world_center").get_to(o.center);
		j.at("world_rotation_quat").get_to(o.rotation);
		j.at("local_size").get_to(o.size);
	}

	void from_json(const nlohmann::json& j, SceneObjectData& s) {
		j.at("name").get_to(s.name);
		j.at("model_name").get_to(s.modelName);
		j.at("srt").get_to(s.transform);

		//なかった場合nullopt
		if (j.contains("obb_collision") && !j.at("obb_collision").is_null()) {
			s.obbCollision = j.at("obb_collision").get<ObbCollision>();
		}
		else {
			s.obbCollision = std::nullopt;
		}
		j.at("is_emissive").get_to(s.isEmissive);
	}

}