#include "AnimationUtils.h"

#include "RailConverter.h"

namespace AnimationUtils {

	std::pair<Transform, Transform> CalculateCurrentTransform(const std::vector<ScalarKeyframe>& scalarKeys, const std::vector<PositionKeyframe>& positionKeys, const std::vector<RotationKeyframe>& rotationKeys, float currentFrame)
	{
		std::pair<Transform, Transform> result;
		float evalTime = AnimationUtils::GetCurrentEvalTime(scalarKeys, currentFrame);

		// 位置キーフレーム
		const auto& posKeys = positionKeys;
		if (!posKeys.empty()) {
			if (evalTime <= posKeys.front().frame) {
				result.first.translate = posKeys.front().value;
			}
			else if (evalTime >= posKeys.back().frame) {
				result.first.translate = posKeys.back().value;
			}
			else {
				auto posIndices = FindKeyframeIndices(posKeys, evalTime);
				const auto& posKey1 = posKeys[posIndices.first];
				const auto& posKey2 = posKeys[posIndices.second];
				float posFrameDiff = posKey2.frame - posKey1.frame;
				float posT = (std::abs(posFrameDiff) < 0.0001f) ? 0.0f : (evalTime - posKey1.frame) / posFrameDiff;
				posT = std::max(0.0f, std::min(1.0f, posT)); // クランプ
				result.first.translate = InterpolatePosition(posKey1, posKey2, posT);
			}
		}
		// 回転キーフレーム
		const auto& rotKeys = rotationKeys;
		if (!rotKeys.empty()) {
			if (evalTime <= rotKeys.front().frame) {
				result.first.rotate = rotKeys.front().value;
			}
			else if (evalTime >= rotKeys.back().frame) {
				result.first.rotate = rotKeys.back().value;
			}
			else {
				auto rotIndices = FindKeyframeIndices(rotKeys, evalTime);
				const auto& rotKey1 = rotKeys[rotIndices.first];
				const auto& rotKey2 = rotKeys[rotIndices.second];
				float rotFrameDiff = rotKey2.frame - rotKey1.frame;
				float rotT = (std::abs(rotFrameDiff) < 0.0001f) ? 0.0f : (evalTime - rotKey1.frame) / rotFrameDiff;
				rotT = std::max(0.0f, std::min(1.0f, rotT));
				result.first.rotate = InterpolateRotation(rotKey1, rotKey2, rotT);
			}
		}

		result.first.UpdateMatrix();
		result.second = RailSystem::RailConverter::ConvertToLeftHand(result.first);
		result.second.UpdateMatrix();

		return result;
	}

	float GetCurrentEvalTime(const std::vector<ScalarKeyframe>& scalarKeys, float currentFrame)
	{
		const auto& keys = scalarKeys;
		auto indices = FindKeyframeIndices(keys, currentFrame);

		const auto& key1 = keys[indices.first];
		const auto& key2 = keys[indices.second];

		if (indices.first == indices.second) {
			return key1.value;
		}

		float t = (currentFrame - key1.frame) / (key2.frame - key1.frame);
		return InterpolateScalar(key1, key2, std::max(0.0f, std::min(1.0f, t)));
	}

	float InterpolateScalar(const ScalarKeyframe& key1, const ScalarKeyframe& key2, float currentFrame)
	{
		if (currentFrame <= 0.0f) return key1.value;
		if (currentFrame >= 1.0f) return key2.value;

		if (key1.interpolation == "BEZIER") {

			Vector2 p0 = { key1.frame, key1.value };
			Vector2 p1 = { key1.handle.right.x, key1.handle.right.y };
			Vector2 p2 = { key2.handle.left.x, key2.handle.left.y };
			Vector2 p3 = { key2.frame, key2.value };

			float t_b = FindBezierTForX(currentFrame, p0, p1, p2, p3);

			return EvaluateBezier(t_b, p0, p1, p2, p3).y;
		}

		return std::lerp(key1.value, key2.value, currentFrame);
	}

	Vector3 InterpolatePosition(const PositionKeyframe& key1, const PositionKeyframe& key2, float currentFrame)
	{
		if (currentFrame <= 0.0f) return key1.value;
		if (currentFrame >= 1.0f) return key2.value;

		return Vector3::Lerp(currentFrame, key1.value, key2.value);
	}

	Quaternion InterpolateRotation(const RotationKeyframe& key1, const RotationKeyframe& key2, float currentFrame)
	{
		if (currentFrame <= 0.0f) return key1.value;
		if (currentFrame >= 1.0f) return key2.value;
		return Quaternion::Slerp(currentFrame, key1.value, key2.value);
	}

	float FindBezierTForX(float targetX, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3)
	{
		float frameRange = p3.x - p0.x;
		if (std::abs(frameRange) < 0.0001f) {
			return 0.0f;
		}

		Vector2 pp0 = { 0.0f, 0.0f };
		Vector2 pp1 = { (p1.x - p0.x) / frameRange, 0.0f };
		Vector2 pp2 = { (p2.x - p0.x) / frameRange, 0.0f };
		Vector2 pp3 = { 1.0f, 0.0f };

		float t_low = 0.0f;
		float t_high = 1.0f;
		float t_guess = targetX;

		for (int i = 0; i < 8; ++i) {
			float x_guess = EvaluateBezier(t_guess, pp0, pp1, pp2, pp3).x;
			float error = x_guess - targetX;

			if (std::abs(error) < 0.001f) {
				break;
			}

			if (error < 0.0f) {
				t_low = t_guess;
			}
			else {
				t_high = t_guess;
			}
			t_guess = (t_high + t_low) * 0.5f;
		}

		return t_guess;
	}

	Vector2 EvaluateBezier(float t, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3)
	{
		float u = 1.0f - t;
		float tt = t * t;
		float uu = u * u;
		float uuu = uu * u;
		float ttt = tt * t;

		float x = uuu * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + ttt * p3.x;
		float y = uuu * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y + ttt * p3.y;

		return { x, y };
	}

}