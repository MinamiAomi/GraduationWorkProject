#pragma once

#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "Math/Transform.h"

namespace AnimationUtils {

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

	std::pair<Transform, Transform>CalculateCurrentTransform(const std::vector<ScalarKeyframe>& scalarKeys, const std::vector<PositionKeyframe>& positionKeys, const std::vector<RotationKeyframe>& rotationKeys, float currentFrame);

	/// <summary>
	/// 指定されたフレームに対応するキーフレームのインデックスペアを探す
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="keys"></param>
	/// <param name="currentFrame"></param>
	/// <returns></returns>
	template <typename T>
	std::pair<size_t, size_t> FindKeyframeIndices(const std::vector<T>& keys, float currentFrame);

	//ヘルパー関数
	float GetCurrentEvalTime(const std::vector<ScalarKeyframe>& scalarKeys, float currentFrame);

	//補間用関数
	float InterpolateScalar(const ScalarKeyframe& key1, const ScalarKeyframe& key2, float currentFrame);
	Vector3 InterpolatePosition(const PositionKeyframe& key1, const PositionKeyframe& key2, float currentFrame);
	Quaternion InterpolateRotation(const RotationKeyframe& key1, const RotationKeyframe& key2, float currentFrame);

	//ベジエ用
	float FindBezierTForX(float targetX, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3);
	Vector2 EvaluateBezier(float t, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3);

	template<typename T>
	std::pair<size_t, size_t> FindKeyframeIndices(const std::vector<T>& keys, float currentFrame)
	{
		if (keys.empty()) return { 0, 0 };

		auto it = std::lower_bound(keys.begin(), keys.end(), currentFrame,
			[](const T& key, float f) {
				return key.frame < f;
			});

		if (it == keys.begin()) {
			return { 0, 0 };
		}

		if (it == keys.end()) {
			return { keys.size() - 1, keys.size() - 1 };
		}

		size_t nextIndex = std::distance(keys.begin(), it);
		size_t prevIndex = nextIndex - 1;

		if (std::abs(keys[prevIndex].frame - currentFrame) < 0.0001f) {
			return { prevIndex, prevIndex };
		}
		if (std::abs(keys[nextIndex].frame - currentFrame) < 0.0001f) {
			return { nextIndex, nextIndex };
		}


		return { prevIndex, nextIndex };
	}

}