#include "AnimationLoader.h"

#include <fstream>
#include <iostream>

#include "File/JsonConverter.h"

#include "AnimationUtils.h"

#ifdef _DEBUG
#include <assert.h>
#endif // _DEBUG

namespace AnimationUtils {

	std::optional<RailSystem::RailAnimation> AnimationLoader::LoadRailAnimation(const std::filesystem::path& filepath)
	{
		std::ifstream file(filepath);
		if (!file.is_open()) {
			std::cerr << "Error: Could not open file " << filepath << std::endl;
#ifdef _DEBUG
			assert(0);
#endif // _DEBUG
			return std::nullopt;
		}

		nlohmann::json data;
		try {
			file >> data;
		}
		catch (nlohmann::json::parse_error& e) {
			std::cerr << "Error: Failed to parse JSON file " << filepath << ". " << e.what() << std::endl;
#ifdef _DEBUG
			assert(0);
#endif // _DEBUG
			return std::nullopt;
		}

		RailSystem::RailAnimation animationData;

		try {
			// メタデータ
			animationData.railMetaData_.startFrame = data["metadata"]["start_frame"].get<int>();
			animationData.railMetaData_.endFrame = data["metadata"]["end_frame"].get<int>();
			animationData.railMetaData_.frameRate = data["metadata"]["frame_rate"].get<float>();

			// キーフレーム
			for (const auto& key : data["curve_eval_time"]) {
				animationData.evalTimeKeys_.push_back(ParseScalarKeyframe(key));
			}
			for (const auto& key : data["rail_world_position"]) {
				animationData.railAnimation_.positionKeys.push_back(ParsePositionKeyframe(key));
			}
			for (const auto& key : data["rail_world_rotation"]) {
				animationData.railAnimation_.rotationKeys.push_back(ParseRotationKeyframe(key));
			}
			for (const auto& key : data["camera_local_position"]) {
				animationData.cameraAnimation_.positionKeys.push_back(ParsePositionKeyframe(key));
			}
			for (const auto& key : data["camera_local_rotation"]) {
				animationData.cameraAnimation_.rotationKeys.push_back(ParseRotationKeyframe(key));
			}

		}
		catch (nlohmann::json::exception& e) {
			std::cerr << "Error: JSON structure mismatch. " << e.what() << std::endl;
#ifdef _DEBUG
			assert(0);
#endif // _DEBUG
			return std::nullopt;
		}

		// データが空でないか基本的なチェック
		if (animationData.evalTimeKeys_.empty() ||
			animationData.railAnimation_.positionKeys.empty() ||
			animationData.railAnimation_.rotationKeys.empty() ||
			animationData.cameraAnimation_.positionKeys.empty() ||
			animationData.cameraAnimation_.rotationKeys.empty()) {
			std::cerr << "Warning: Animation data contains empty keyframe tracks." << std::endl;
		}

		return animationData;
	}

	std::optional<std::tuple<std::vector<ScalarKeyframe>, std::vector< PositionKeyframe>, std::vector<RotationKeyframe >>> AnimationLoader::LoadAnimation(const nlohmann::json& data)
	{
		std::vector<ScalarKeyframe> evalTimeKeys;
		std::vector<PositionKeyframe> positionKeys;
		std::vector<RotationKeyframe> rotationKeys;

		try {

				if (data.contains("curve_eval_time")) {
					data.at("curve_eval_time").get_to(evalTimeKeys);
				}

				if (data.contains("position")) {
					data.at("position").get_to(positionKeys);
				}

				if (data.contains("rotation")) {
					data.at("rotation").get_to(rotationKeys);
				}
		}
		catch (nlohmann::json::exception& e) {
			std::cerr << "Error: JSON structure mismatch in Animation Data. " << e.what() << std::endl;
			return std::nullopt;
		}

		// データが空なら警告だけ出して、空のタプルを返すか、nulloptにするかは仕様次第
		if (evalTimeKeys.empty() && positionKeys.empty() && rotationKeys.empty()) {
			// データが全くない場合は nullopt を返す例
			return std::nullopt;
		}

		return std::make_tuple(evalTimeKeys, positionKeys, rotationKeys);
	}

	AnimationUtils::ScalarKeyframe AnimationLoader::ParseScalarKeyframe(const nlohmann::json& key)
	{
		AnimationUtils::ScalarKeyframe keyframe;
		keyframe.frame = key["frame"].get<float>();
		keyframe.value = key["value"].get<float>();


		// 補間タイプを読み込む（存在しない場合はLINEAR）
		keyframe.interpolation = key.value("interpolation", "LINEAR");

		// 常にハンドルにデフォルト値を設定 (key.frame, key.value)
		// これにより、BEZIERではないキーも有効なハンドル値を持つ
		keyframe.handle.left = { keyframe.frame, keyframe.value };
		keyframe.handle.right = { keyframe.frame, keyframe.value };

		// BEZIER補間の場合、JSONからハンドル情報を読み込んで上書き
		if (keyframe.interpolation == "BEZIER") {
			if (key.contains("handle_left")) {
				keyframe.handle.left.x = key["handle_left"]["x"].get<float>();
				keyframe.handle.left.y = key["handle_left"]["y"].get<float>();
			}
			if (key.contains("handle_right")) {
				keyframe.handle.right.x = key["handle_right"]["x"].get<float>();
				keyframe.handle.right.y = key["handle_right"]["y"].get<float>();
			}
		}

		return keyframe;
	}

	AnimationUtils::PositionKeyframe AnimationLoader::ParsePositionKeyframe(const nlohmann::json& key)
	{
		AnimationUtils::PositionKeyframe keyframe;
		keyframe.frame = key["frame"].get<float>();
		keyframe.value.x = key["value"]["x"].get<float>();
		keyframe.value.y = key["value"]["y"].get<float>();
		keyframe.value.z = key["value"]["z"].get<float>();
		return keyframe;
	}

	AnimationUtils::RotationKeyframe AnimationLoader::ParseRotationKeyframe(const nlohmann::json& key)
	{
		AnimationUtils::RotationKeyframe keyframe;
		keyframe.frame = key["frame"].get<float>();
		keyframe.value.w = key["value"]["w"].get<float>();
		keyframe.value.x = key["value"]["x"].get<float>();
		keyframe.value.y = key["value"]["y"].get<float>();
		keyframe.value.z = key["value"]["z"].get<float>();
		return keyframe;
	}

	void from_json(const nlohmann::json& j, AnimationUtils::ScalarKeyframe& k) {
		j.at("frame").get_to(k.frame);
		j.at("value").get_to(k.value);
		if (j.contains("interpolation")) j.at("interpolation").get_to(k.interpolation);
	}

	void from_json(const nlohmann::json& j, AnimationUtils::PositionKeyframe& k) {
		j.at("frame").get_to(k.frame);

		const auto& val = j.at("value");
		k.value.x = val.at("x").get<float>();
		k.value.y = val.at("y").get<float>();
		k.value.z = val.at("z").get<float>();

		if (j.contains("interpolation")) j.at("interpolation").get_to(k.interpolation);
	}

	void from_json(const nlohmann::json& j, AnimationUtils::RotationKeyframe& k) {
		j.at("frame").get_to(k.frame);

		const auto& val = j.at("value");
		k.value.w = val.at("w").get<float>();
		k.value.x = val.at("x").get<float>();
		k.value.y = val.at("y").get<float>();
		k.value.z = val.at("z").get<float>();

		if (j.contains("interpolation")) j.at("interpolation").get_to(k.interpolation);
	}
}