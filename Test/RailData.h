#pragma once
#pragma once

#include <vector>
#include <string>

#include "Math/MathUtils.h"

namespace RailSystem {

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

	struct RailMetaData {
		int startFrame;
		int endFrame;
		float frameRate;
	};

	/// <summary>
	/// Jsonファイル全体を保存する
	/// </summary>
	
	struct  NodeAnimation {
		std::vector<PositionKeyframe> positionKeys;
		std::vector<RotationKeyframe> rotationKeys;
	};
	class RailAnimation {
	public:
		RailMetaData railMetaData_;
		std::vector<ScalarKeyframe> evalTimeKeys_;
		NodeAnimation railAnimation_;
		NodeAnimation cameraAnimation_;
	};


}