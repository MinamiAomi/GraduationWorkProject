#include "SceneObjectData.h"
#include "File/JsonConverter.h"

namespace SceneObjectSystem {

	void from_json(const nlohmann::json& j, CapsuleCollisionData& o) {
		j.at("center").get_to(o.center);
		j.at("radius").get_to(o.radius);
		j.at("height").get_to(o.height);
		j.at("quaternion").get_to(o.quaternion);
	}

	void from_json(const nlohmann::json& j, SphereCollisionData& o)
	{
		j.at("center").get_to(o.center);
		j.at("radius").get_to(o.radius);
	}

	void from_json(const nlohmann::json& j, SceneObjectData& s) {
		if (j.contains("name") && !j.at("name").is_null()) {
			s.name = j.at("name").get<std::string>();
		}
		if (j.contains("model_name") && !j.at("model_name").is_null()) {
			s.modelName = j.at("model_name").get<std::string>();
		}
		j.at("srt").get_to(s.transform);
		j.at("type").get_to(s.type);

		s.capsuleCollisionData = std::nullopt;
		s.sphereCollisionData = std::nullopt;
		
		if (j.contains("collider") && !j.at("collider").is_null()) {
			if (j.contains("collider") && !j.at("collider").is_null()) {

				const auto& colJson = j.at("collider");

				if (colJson.contains("type")) {
					std::string type = colJson.at("type").get<std::string>();

					if (type == "SPHERE") {
						s.sphereCollisionData = colJson.get<SphereCollisionData>();
					}
					else if (type == "CAPSULE") {
						s.capsuleCollisionData = colJson.get<CapsuleCollisionData>();
					}
				}
			}
		}

		s.pointLightData = std::nullopt;
		s.eventTriggerTypeData = std::nullopt;

		switch (s.type) {
		case ObjectType::PointLight:
			if (j.contains("point_light") && !j.at("point_light").is_null()) {
				s.pointLightData = j.at("point_light").get<PointLightData>();
			}
			break;

		case ObjectType::EventTrigger:
			if (j.contains("trigger_type") && !j.at("trigger_type").is_null()) {
				s.eventTriggerTypeData = j.at("trigger_type").get<EventTriggerTypeObject>();
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
	void from_json(const nlohmann::json& j, EventTriggerTypeObject& p)
	{
		p.enemyTrigger = std::nullopt;
		p.eventTrigger = std::nullopt;

		if (!j.contains("trigger_type") || j.at("trigger_type").is_null()) {
			return;
		}

		std::string type = j.at("trigger_type").get<std::string>();

		if (!j.contains("properties") || j.at("properties").is_null()) {
			return;
		}

		const auto& props = j.at("properties");

		if (type == "ENEMY") {
			p.enemyTrigger = props.get<EnemyTriggerData>();
		}
		else if (type == "GIMMICK") {
			p.eventTrigger = props.get<EventTriggerData>();
		}
	}
	void from_json(const nlohmann::json& j, EnemyTriggerData& p)
	{
		j.at("enemy_count").get_to(p.enemyCount);
	}
	void from_json(const nlohmann::json& j, EventTriggerData& p)
	{
		j.at("gimmick_distance").get_to(p.distance);
	}
}