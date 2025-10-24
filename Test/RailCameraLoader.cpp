#include "RailCameraLoader.h"

#include <fstream>
#include <iostream>

#include "Externals/nlohmann/json.hpp"
namespace RailCameraSystem {
	static Vector2 ParseVec2(const nlohmann::json& json, const std::string& key) {
		Vector2 result;
		if (json.contains(key) && json[key].is_object()) {
			result.x = json[key].value("x", 0.0f);
			result.y = json[key].value("y", 0.0f);
		}
		return result;
	}


	std::optional<RailCameraAnimation> RailCameraSystem::AnimationLoader::LoadAnimation(const std::filesystem::path& filepath)
	{
		//ファイルを開く
		std::ifstream fs(filepath);

		//開けなかった場合エラー文とnullopt
		if (!fs.is_open()) {
			std::cerr << "Error : Failed to open animation file" << std::endl;
			return std::nullopt;
		}

		try {
			nlohmann::json data = nlohmann::json::parse(fs);
			RailCameraAnimation animationData;

			//メタデータの読み込み
			if (data.contains("metadata") && data["metadata"].is_object()) {
				const auto& meta = data["metadata"];
				animationData.railCameraMetaData_.startFrame = meta.value("start_frame", 0);
				animationData.railCameraMetaData_.endFrame = meta.value("end_frame", 0);
				animationData.railCameraMetaData_.frameRate = meta.value("frame_rate", 0.0f);
			}

			//評価時間のキーフレームの読み込み
			if (data.contains("curve_eval_time") && data["curve_eval_time"].is_array()) {
				for (const auto& keyJson : data["curve_eval_time"]) {
					ScalarKeyframe key;
					key.frame = keyJson.value("frame", 0.0f);
					key.value = keyJson.value("value", 0.0f);
					key.interpolation = keyJson.value("interpolation", "LINEAR");
					//ベジエなら格納
					if (key.interpolation == "BEZIER") {
						key.handle.left = ParseVec2(keyJson, "handle_left");
						key.handle.right = ParseVec2(keyJson, "handle_right");
					}
					animationData.scalarKeys_.push_back(key);
				}
			}

			//位置キーフレームの読み込み
			if (data.contains("camera_location_keyframes") && data["camera_location_keyframes"].is_array()) {
				for (const auto& keyJson : data["camera_location_keyframes"]) {
					PositionKeyframe key;
					key.frame = keyJson.value("frame", 0.0f);
					key.interpolation = keyJson.value("interpolation", "LINEAR");


					if (keyJson.contains("value") && keyJson["value"].is_object()) {
						key.value.x = keyJson["value"].value("x", 0.0f);
						key.value.y = keyJson["value"].value("y", 0.0f);
						key.value.z = keyJson["value"].value("z", 0.0f);
					}

					if (key.interpolation == "BEZIER" && keyJson.contains("handles") && keyJson["handles"].is_object()) {
						const auto& handles = keyJson["handles"];
						if (handles.contains("x")) {
							key.handles.x.left = ParseVec2(handles["x"], "left");
							key.handles.x.right = ParseVec2(handles["x"], "right");
						}
						if (handles.contains("y")) {
							key.handles.y.left = ParseVec2(handles["y"], "left");
							key.handles.y.right = ParseVec2(handles["y"], "right");
						}
						if (handles.contains("z")) {
							key.handles.z.left = ParseVec2(handles["z"], "left");
							key.handles.z.right = ParseVec2(handles["z"], "right");
						}
					}

					animationData.positionKeys_.push_back(key);
				}

				//回転キーフレームの読み込み
				if (data.contains("camera_rotation_keyframes") && data["camera_rotation_keyframes"].is_array()) {
					for (const auto& keyJson : data["camera_rotation_keyframes"]) {

						RotationKeyframe key;
						key.frame = keyJson.value("frame", 0.0f);
						if (keyJson.contains("value") && keyJson["value"].is_object()) {
							key.value.w = keyJson["value"].value("w", 0.0f);
							key.value.x = keyJson["value"].value("x", 0.0f);
							key.value.y = keyJson["value"].value("y", 0.0f);
							key.value.z = keyJson["value"].value("z", 0.0f);
						}
						animationData.rotationKeys_.push_back(key);
					}
				}
			}
			return animationData;
		}
		//パース出来なかったった場合の例外処理nulloptを返す
		catch (nlohmann::json::parse_error& e) {
			std::cerr << "Json parse error" << e.what() << std::endl;
			return std::nullopt;
		}

	}


}
