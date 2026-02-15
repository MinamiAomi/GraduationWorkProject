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

		const Transform& GetTransform() const { return convertRailTransform_; }

		void Play();

		void Pause();

		void Stop();

		void Loop();

		bool IsPlaying() const { return isPlaying_; }

		bool IsFinished() const;

		float GetRealSpeed() const { return realSpeed_; }

		void SetPlaybackSpeed(float speed) { playbackSpeed_ = speed; }
		float GetPlaybackSpeed() const { return playbackSpeed_; }

		const RailAnimation* GetRailAnimationDate() const {return animationData_.get();}

		void SetCurrentFrame(float frame);
		float GetCurrentFrame() const;

		// 全部ワールドです
		// レールの指定されてフレームの座標
		Vector3 EvaluateRailPosition(float frame) const;
		// レールの指定したフレームにおける回転
		Quaternion EvaluateRailRotation(float frame) const;
		// レールの指定したフレームにおけるTransform
		Transform EvaluateRailTransform(float frame) const;

		// カメラの指定されてフレームの座標
		Vector3 EvaluateLocalCameraPosition(float frame) const;
		// カメラの指定したフレームにおける回転
		Quaternion EvaluateLocalCameraRotation(float frame) const;
		// カメラの指定したフレームにおけるTransform
		Transform EvaluateLocalCameraTransform(float frame) const;

		// カメラの指定されてフレームの座標
		Vector3 EvaluateWorldCameraPosition(float frame) const;
		// カメラの指定したフレームにおける回転
		Quaternion EvaluateWorldCameraRotation(float frame) const;
		// カメラの指定したフレームにおけるTransform
		Transform EvaluateWorldCameraTransform(float frame) const;
		
#ifdef _DEBUG
		void DrawImGui();
#endif // _DEBUG


		std::shared_ptr<const RailAnimation> animationData_;
		float currentFrame_;
		float totalDurationFrames_;
		bool isPlaying_;
		float playbackSpeed_;

		//生データ
		Transform railTransform_;
		//左手座標系に変換された後
		Transform convertRailTransform_;
		Vector3 preCameraPosition_;
		float realSpeed_;
		float level1GoalFrame_;
		float level2GoalFrame_;
	};

}