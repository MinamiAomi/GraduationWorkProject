#include "RailCameraLoader.h"

#include <fstream>
#include <iostream>

namespace RailCameraSystem {
	/*static Vector2 ParseVec2(const nlohmann::json& json, const std::string& key) {
		Vector2 result;
		if (json.contains(key) && json[key].is_object()) {
			result.x = json[key].value("x", 0.0f);
			result.y = json[key].value("y", 0.0f);
		}
		return result;
	}*/


	std::optional<RailCameraAnimation> RailCameraSystem::AnimationLoader::LoadAnimation(const std::filesystem::path& filepath)
	{
		std::ifstream file(filepath);
		if (!file.is_open()) {
			std::cerr << "Error: Could not open file " << filepath << std::endl;
			return std::nullopt;
		}

		nlohmann::json data;
		try {
			file >> data;
		}
		catch (nlohmann::json::parse_error& e) {
			std::cerr << "Error: Failed to parse JSON file " << filepath << ". " << e.what() << std::endl;
			return std::nullopt;
		}

		RailCameraAnimation animationData;

		try {
			// メタデータ
			animationData.railCameraMetaData_.startFrame = data["metadata"]["start_frame"].get<int>();
			animationData.railCameraMetaData_.endFrame = data["metadata"]["end_frame"].get<int>();
			animationData.railCameraMetaData_.frameRate = data["metadata"]["frame_rate"].get<float>();

			// キーフレーム
			for (const auto& key : data["curve_eval_time"]) {
				animationData.evalTimeKeys_.push_back(ParseScalarKeyframe(key));
			}
			for (const auto& key : data["camera_location_keyframes"]) {
				animationData.positionKeys_.push_back(ParsePositionKeyframe(key));
			}
			for (const auto& key : data["camera_rotation_keyframes"]) {
				animationData.rotationKeys_.push_back(ParseRotationKeyframe(key));
			}

		}
		catch (nlohmann::json::exception& e) {
			std::cerr << "Error: JSON structure mismatch. " << e.what() << std::endl;
			return std::nullopt;
		}

		// データが空でないか基本的なチェック
		if (animationData.evalTimeKeys_.empty() || animationData.positionKeys_.empty() || animationData.rotationKeys_.empty()) {
			std::cerr << "Warning: Animation data contains empty keyframe tracks." << std::endl;
		}

		return animationData;
	}

	ScalarKeyframe AnimationLoader::ParseScalarKeyframe(const nlohmann::json& key)
	{
		ScalarKeyframe keyframe;
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

	PositionKeyframe AnimationLoader::ParsePositionKeyframe(const nlohmann::json& key)
	{
		PositionKeyframe keyframe;
		keyframe.frame = key["frame"].get<float>();
		keyframe.value.x = key["value"]["x"].get<float>();
		keyframe.value.y = key["value"]["y"].get<float>();
		keyframe.value.z = key["value"]["z"].get<float>();
		return keyframe;
	}

	RotationKeyframe AnimationLoader::ParseRotationKeyframe(const nlohmann::json& key)
	{
		RotationKeyframe keyframe;
		keyframe.frame = key["frame"].get<float>();
		keyframe.value.w = key["value"]["w"].get<float>();
		keyframe.value.x = key["value"]["x"].get<float>();
		keyframe.value.y = key["value"]["y"].get<float>();
		keyframe.value.z = key["value"]["z"].get<float>();
		return keyframe;
	}


}
