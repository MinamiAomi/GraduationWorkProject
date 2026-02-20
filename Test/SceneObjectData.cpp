#include "SceneObjectData.h"
#include "File/JsonConverter.h"

#include "AnimationLoader.h"

#include "SceneObjectConverter.h"

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

		s.pointLightData = std::nullopt;
		s.enemySpawnData = std::nullopt;
		s.gimmickMovers = std::nullopt;
		s.gimmickTriggers = std::nullopt;
		s.obstacles = std::nullopt;

		switch (s.type) {
		case ObjectType::PointLight:
			if (j.contains("point_light") && !j.at("point_light").is_null()) {
				s.pointLightData = j.at("point_light").get<PointLightData>();
			}
			break;

		case ObjectType::EnemySpawn:
			if (j.contains("enemy_spawn_data") && !j.at("enemy_spawn_data").is_null()) {
				s.enemySpawnData = j.at("enemy_spawn_data").get<EnemySpawnData>();
			}
			break;

		case ObjectType::Gimmick:
			if (j.contains("gimmick_data") && !j.at("gimmick_data").is_null()) {

				const auto& gimmickJson = j.at("gimmick_data");

				if (!gimmickJson.contains("subtype")) return;

				std::string subtype = gimmickJson.at("subtype").get<std::string>();

				if (subtype == "TRIGGER") {
					s.gimmickTriggers = gimmickJson.get<GimmickTriggerData>();
				}
				else if (subtype == "MOVER") {
					s.gimmickMovers = gimmickJson.get<GimmickMoverData>();
				}
				else if (subtype == "POINTLIGHT") {
					s.gimmickPointlights = gimmickJson.get<GimmickPointLightData>();

				}
			}
			break;
		case ObjectType::Obstacle:
			if (j.contains("obstacle_data") && !j.at("obstacle_data").is_null()) {
				s.obstacles = j.at("obstacle_data").get<ObstacleData>();
			}
			break;
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

	void from_json(const nlohmann::json& j, EnemySpawnData& p) {
		// JSONキー修正: "formation"
		if (j.contains("formation")) {
			p.formation = j.at("formation").get<std::vector<std::vector<bool>>>();
		}
		if (j.contains("isOnce")) {
			j.at("isOnce").get_to(p.isOnce);
		}
	}
	void from_json(const nlohmann::json& j, GimmickTriggerData& p)
	{
		if (!j.contains("subtype")) return;

		std::string subtype = j.at("subtype").get<std::string>();

		if (subtype == "TRIGGER") {

			if (j.contains("key")) j.at("key").get_to(p.key);
			if (j.contains("isOnce")) j.at("isOnce").get_to(p.isOnce);
		}
	}

	void from_json(const nlohmann::json& j, GimmickMoverData& p)
	{
		if (!j.contains("subtype")) return;

		std::string subtype = j.at("subtype").get<std::string>();

		if (subtype == "MOVER") {

			if (j.contains("key")) j.at("key").get_to(p.key);
			if (j.contains("duration")) j.at("duration").get_to(p.duration);
			if (j.contains("is_cyclic")) j.at("is_cyclic").get_to(p.isCyclic);

			auto result = AnimationUtils::AnimationLoader::LoadAnimation(j);

			p.evalTimeKeys = std::get<0>(result.value());
			p.moverAnimation.positionKeys = std::get<1>(result.value());
			p.moverAnimation.rotationKeys = std::get<2>(result.value());

		}
	}

	void from_json(const nlohmann::json& j, GimmickPointLightData& p)
	{
		if (j.contains("key")) j.at("key").get_to(p.gimmickMoverData.key);
		if (j.contains("duration")) j.at("duration").get_to(p.gimmickMoverData.duration);
		if (j.contains("is_cyclic")) j.at("is_cyclic").get_to(p.gimmickMoverData.isCyclic);

		auto result = AnimationUtils::AnimationLoader::LoadAnimation(j);
		if (result.has_value()) {
			p.gimmickMoverData.evalTimeKeys = std::get<0>(result.value());
			p.gimmickMoverData.moverAnimation.positionKeys = std::get<1>(result.value());
			p.gimmickMoverData.moverAnimation.rotationKeys = std::get<2>(result.value());
		}

		if (j.contains("light_params") && !j.at("light_params").is_null()) {
			const auto& lp = j.at("light_params");

			p.pointlightData = lp.get<PointLightData>();

			p.pointlightData.isActive = true;
		}
	}

	void from_json(const nlohmann::json& j, ObstacleData& p)
	{
		if (j.contains("hp")) j.at("hp").get_to(p.hp);
	}

	void GimmickMoverObject::Update()
	{

		if (isActive) {
			time++;
			auto result = AnimationUtils::CalculateCurrentTransform(evalTimeKeys, moverAnimation.positionKeys, moverAnimation.rotationKeys, time);
			transform.translate = SceneObjectSystem::SceneObjectConverter::ConvertTranslateToLeftHand(result.first.translate);
			transform.rotate = SceneObjectSystem::SceneObjectConverter::ConvertRotateToLeftHand(result.first.rotate);
			transform.UpdateMatrix();
			model.SetWorldMatrix(transform.worldMatrix);
			if (time >= duration && isCyclic) {
				time = 0.0f;
			}
			else if (time >= duration && !isCyclic) {
				time = duration;
			}
		}

	}
	void PointLightObject::Update()
	{
		transform.UpdateMatrix();
		model.SetWorldMatrix(transform.worldMatrix);
		powerEmitter_.Update();
		lightObject.Update();
	}
	void GimmickPointLightObject::Update()
	{
		mover.Update(transform);
		pointlight.Update();
		transform.UpdateMatrix();

		collider->center = transform.translate;
		model.SetWorldMatrix(transform.worldMatrix);

	}
	void GimmickPointLightObject::Pointlight::Update()
	{
		lightObject.Update();
		powerEmitter.Update();
	}
	void GimmickPointLightObject::GimmickMover::Update(Transform& transform)
	{
		if (isActive) {
			time++;
			auto result = AnimationUtils::CalculateCurrentTransform(evalTimeKeys, moverAnimation.positionKeys, moverAnimation.rotationKeys, time);
			transform.translate = SceneObjectSystem::SceneObjectConverter::ConvertTranslateToLeftHand(result.first.translate);
			transform.rotate = SceneObjectSystem::SceneObjectConverter::ConvertRotateToLeftHand(result.first.rotate);
			transform.UpdateMatrix();
			if (time >= duration && isCyclic) {
				time = 0.0f;
			}
			else if (time >= duration && !isCyclic) {
				time = duration;
			}
		}
	}
	void ObstacleObject::Update()
	{
		transform.translate = basePosition;
		transform.UpdateMatrix();
		model.SetWorldMatrix(transform.worldMatrix);
	}
	void ObstacleObject::SetDamage()
	{
		hp--;
		if (hp < 0) {
			isAlive = false;
			model.SetIsActive(false);
		}
	}
}