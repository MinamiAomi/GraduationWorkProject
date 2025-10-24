#include "RailCameraController.h"

#include <cmath>
#include <algorithm>
#include <stdexcept>

RailCameraSystem::RailCameraController::RailCameraController(std::shared_ptr<const RailCameraSystem::RailCameraAnimation> animationData)
{
	if (!animationData) {
		throw std::invalid_argument("AnimationData cannot be null.");
	}
	animationData_ = animationData;
	isPlaying_ = false;
	//メタデータからアニメーションの開始と終了の位置を取得
	currentFrame_ = float(animationData_->railCameraMetaData_.startFrame);
	totalDurationFrames_ = float(animationData_->railCameraMetaData_.endFrame);
}

void RailCameraSystem::RailCameraController::Update(float deltaTime)
{
	//止められているor終了していたら再生しない
	if (!isPlaying_ || IsFinished()) {
		return;
	}

	//デルタタイムをフレームの進みに変換
	float frameIncrement = deltaTime * animationData_->railCameraMetaData_.frameRate;
	currentFrame_ += frameIncrement;

	//アニメーションの終了フレームを超えないようにクランプ
	if (currentFrame_ > totalDurationFrames_) {
		currentFrame_ = totalDurationFrames_;

		//いったんループ
		Loop();
	}
}

Transform RailCameraSystem::RailCameraController::GetCurrentTransform() const
{

	Transform transform;

	if (!animationData_) {
		return transform;
	}

	Vector3 currentPosition = Vector3::zero;
	if (!animationData_->positionKeys_.empty()) {
		auto posIndices = FindKeyframeIndices(animationData_->positionKeys_, currentFrame_);
		currentPosition = InterpolatePosition(animationData_->positionKeys_.at(posIndices.first), animationData_->positionKeys_.at(posIndices.second), currentFrame_);
	}

	Quaternion currentRotation = Quaternion::identity;
	if (!animationData_->rotationKeys_.empty()) {
		auto rotateIndices = FindKeyframeIndices(animationData_->rotationKeys_, currentFrame_);
		currentRotation = InterpolateRotation(animationData_->rotationKeys_.at(rotateIndices.first), animationData_->rotationKeys_.at(rotateIndices.second), currentFrame_);
	}

	transform.translate = currentPosition;
	transform.rotate = currentRotation;
	return  transform;
}

void RailCameraSystem::RailCameraController::Play()
{
	isPlaying_ = true;
}

void RailCameraSystem::RailCameraController::Pause()
{
	isPlaying_ = false;
}

void RailCameraSystem::RailCameraController::Stop()
{
	isPlaying_ = false;
	currentFrame_ = static_cast<float>(animationData_->railCameraMetaData_.startFrame);
}

void RailCameraSystem::RailCameraController::Loop()
{
	isPlaying_ = true;
	currentFrame_ = static_cast<float>(animationData_->railCameraMetaData_.startFrame);
}

template<typename T>
inline std::pair<size_t, size_t> RailCameraSystem::RailCameraController::FindKeyframeIndices(const std::vector<T>& keys, float currentFrame) const
{
	if (keys.empty()) {
		return { 0, 0 };
	}
	if (currentFrame <= keys.front().frame) {
		return { 0, 0 };
	}
	if (currentFrame >= keys.back().frame) {
		return { keys.size() - 1, keys.size() - 1 };
	}

	// 二分探索などで高速化も可能だが、ここでは線形探索で実装
	for (size_t i = 0; i < keys.size() - 1; ++i) {
		if (currentFrame >= keys[i].frame && currentFrame <= keys[i + 1].frame) {
			return { i, i + 1 };
		}
	}
	return { keys.size() - 1, keys.size() - 1 };
}

float RailCameraSystem::RailCameraController::InterpolateScalar(const RailCameraSystem::ScalarKeyframe& key1, const RailCameraSystem::ScalarKeyframe& key2, float currentFrame) const
{
	if (key1.frame == key2.frame) { return key1.value; }
	float t = (currentFrame - key1.frame) / (key2.frame - key1.frame);


	return std::lerp(key1.value, key2.value, t);
}

Vector3 RailCameraSystem::RailCameraController::InterpolatePosition(const RailCameraSystem::PositionKeyframe& key1, const RailCameraSystem::PositionKeyframe& key2, float currentFrame) const
{
	if (key1.frame == key2.frame) { return key1.value; }
	float t = (currentFrame - key1.frame) / (key2.frame - key1.frame);

	return Vector3::Lerp(t, key1.value, key2.value);
}

Quaternion RailCameraSystem::RailCameraController::InterpolateRotation(const RailCameraSystem::RotationKeyframe& key1, const RailCameraSystem::RotationKeyframe& key2, float currentFrame) const
{
	if (key1.frame == key2.frame) { return key1.value; }
	float t = (currentFrame - key1.frame) / (key2.frame - key1.frame);


	return Quaternion::Slerp(t, key1.value, key2.value);
}
