#pragma once

#include <vector>
#include <memory>

#include "RailCameraData.h"
#include "Math/Transform.h"


namespace RailCameraSystem {

	class AnimationController {
	public:
		explicit AnimationController(std::shared_ptr<const RailCameraSystem::RailCameraAnimation>animationData);

		void Update(float deltaTime);

		Transform GetCurrentTransform() const;

		void Play();

		void Pause();

		void Stop();

		bool IsPlaying() const { return isPlaying_; }

		bool IsFinished() const { return currentFrame_ >= totalDurationFrames_; }
	private:
		/// <summary>
		/// 指定されたフレームに対応するキーフレームのインデックスペアを探す
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="keys"></param>
		/// <param name="currentFrame"></param>
		/// <returns></returns>
		template <typename T>
		std::pair<size_t, size_t> FindKeyframeIndices(const std::vector<T>& keys, float currentFrame) const;

		//補間用関数
		float InterpolateScalar(const RailCameraSystem::ScalarKeyframe& key1, const RailCameraSystem::ScalarKeyframe& key2, float currentFrame) const;
		Vector3 InterpolatePosition(const RailCameraSystem::PositionKeyframe& key1, const RailCameraSystem::PositionKeyframe& key2, float currentFrame) const;
		Quaternion InterpolateRotation(const RailCameraSystem::RotationKeyframe& key1, const RailCameraSystem::RotationKeyframe& key2, float currentFrame) const;

		std::shared_ptr<const RailCameraAnimation> animationData_;
		float currentFrame_;
		float totalDurationFrames_;
		bool isPlaying_;
	};

}