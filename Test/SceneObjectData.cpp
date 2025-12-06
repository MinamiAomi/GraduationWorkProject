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
		j.at("type").get_to(s.type);

		if (j.contains("capsule_collision") && !j.at("capsule_collision").is_null()) {
			s.capsuleCollisionData = j.at("capsule_collision").get<CapsuleCollisionData>();
		}
		else {
			s.capsuleCollisionData = std::nullopt;
		}


		s.pointLightData = std::nullopt;
		s.enemyData = std::nullopt;
		s.emitterData = std::nullopt;

		switch (s.type) {
		case ObjectType::PointLight:
			if (j.contains("point_light") && !j.at("point_light").is_null()) {
				s.pointLightData = j.at("point_light").get<PointLightData>();
			}
			break;

		case ObjectType::Enemy:
			if (j.contains("enemy_data") && !j.at("enemy_data").is_null()) {
				s.enemyData = j.at("enemy_data").get<EnemyData>();
			}
			break;

		case ObjectType::Emitter:
			if (j.contains("emitter_data") && !j.at("emitter_data").is_null()) {
				s.emitterData = j.at("emitter_data").get<EmitterData>();
			}
			break;

		case ObjectType::Unknown:
		default:
			break;
		}
	}

	void from_json(const nlohmann::json& j, PointLightData& p)
	{
		j.at("offset").get_to(p.offset);
		j.at("color").get_to(p.color);
		j.at("intensity").get_to(p.intensity);
		j.at("range").get_to(p.range);
		j.at("decay").get_to(p.decay);
	}
	void from_json(const nlohmann::json& j, EmitterData& p)
	{
		j, p;
	}
	void from_json(const nlohmann::json& j, EnemyData& p)
	{
		j, p;
	}
}