#pragma once

#include <vector>
#include <memory>

#include "RailData.h"
#include "Math/Transform.h"


namespace RailSystem {

	class RailAnimationPlayer {
	public:
		explicit RailAnimationPlayer(std::shared_ptr<const RailSystem::RailAnimation>animationData);

		void Update(float deltaTime);

		const Transform& GetTransform() const { return convertTransform_; }

		void Play();

		void Pause();

		void Stop();

		void Loop();

		bool IsPlaying() const { return isPlaying_; }

		bool IsFinished() const { return currentFrame_ >= totalDurationFrames_; }

		float GetRealSpeed() const { return realSpeed_; }

		void SetPlaybackSpeed(float speed) { playbackSpeed_ = speed; }
		float GetPlaybackSpeed() const { return playbackSpeed_; }

		const RailAnimation* GetRailAnimationDate() const {return animationData_.get();}

		void SetCurrentFrame(int frame);
		float GetCurrentFrame() const;

		//指定されてフレームの座標
		Vector3 EvaluatePosition(float frame) const;
		// 指定したフレームにおける回転
		Quaternion EvaluateRotation(float frame) const;
		//指定したフレームにおけるTransform
		Transform EvaluateTransform(float frame) const;
	private:

		void CalculateCurrentTransform();
		/// <summary>
		/// 指定されたフレームに対応するキーフレームのインデックスペアを探す
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="keys"></param>
		/// <param name="currentFrame"></param>
		/// <returns></returns>
		template <typename T>
		std::pair<size_t, size_t> FindKeyframeIndices(const std::vector<T>& keys, float currentFrame) const;

		//ヘルパー関数
		float GetCurrentEvalTime() const;

		//補間用関数
		float InterpolateScalar(const RailSystem::ScalarKeyframe& key1, const RailSystem::ScalarKeyframe& key2, float currentFrame) const;
		Vector3 InterpolatePosition(const RailSystem::PositionKeyframe& key1, const RailSystem::PositionKeyframe& key2, float currentFrame) const;
		Quaternion InterpolateRotation(const RailSystem::RotationKeyframe& key1, const RailSystem::RotationKeyframe& key2, float currentFrame) const;

		//ベジエ用
		float FindBezierTForX(float targetX, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3) const;
		Vector2 EvaluateBezier(float t, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3) const;

#ifdef _DEBUG
		void DrawImGui();
#endif // _DEBUG


		std::shared_ptr<const RailAnimation> animationData_;
		float currentFrame_;
		float totalDurationFrames_;
		bool isPlaying_;
		float playbackSpeed_;

		//生データ
		Transform transform_;
		//左手座標系に変換された後
		Transform convertTransform_;
		Vector3 preCameraPosition_;
		float realSpeed_;
	};

}