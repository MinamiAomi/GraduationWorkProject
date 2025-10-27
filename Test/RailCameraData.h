#pragma once
#pragma once

#include <vector>
#include <string>

#include "Math/MathUtils.h"

namespace RailCameraSystem {

	/// <summary>
	/// ベジエ用のハンドル
	/// </summary>
	struct BezierHandle
	{
		Vector2 left;
		Vector2 right;
	};

	/// <summary>
	/// float格納用
	/// </summary>
	struct ScalarKeyframe {
		float frame;
		float value;
		std::string interpolation;
		BezierHandle handle;
	};

	/// <summary>
	/// 位置
	/// </summary>
	struct PositionKeyframe {
		float frame;
		Vector3 value;
		std::string interpolation;
		struct {
			BezierHandle x, y, z;
		} handles;
	};

	/// <summary>
	/// 回転
	/// </summary>
	struct RotationKeyframe {
		float frame;
		Quaternion value;
		std::string interpolation;
	};

	struct RailCameraMetaData {
		int startFrame;
		int endFrame;
		float frameRate;
	};

	/// <summary>
	/// Jsonファイル全体を保存する
	/// </summary>
	class RailCameraAnimation {
	public:
		RailCameraMetaData railCameraMetaData_;

		std::vector<ScalarKeyframe> evalTimeKeys_;
		std::vector<PositionKeyframe> positionKeys_;
		std::vector<RotationKeyframe> rotationKeys_;
	};
}