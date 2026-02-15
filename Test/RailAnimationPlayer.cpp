#include "RailAnimationPlayer.h"

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <iostream> 

#include "Engine/File/JsonConverter.h"
#include "RailConverter.h"

#include "AnimationUtils.h"

#include "LevelManager.h"
#ifdef _DEBUG
#include <iomanip>
#include <sstream>
#include <Windows.h>

#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

RailSystem::RailAnimationPlayer::RailAnimationPlayer(std::shared_ptr<const RailSystem::RailAnimation> animationData)
{
	if (!animationData) {
		throw std::invalid_argument("AnimationData cannot be null.");
	}
	animationData_ = animationData;
	isPlaying_ = false;

	//メタデータからアニメーションの開始と終了の位置を取得
	currentFrame_ = float(animationData_->railMetaData_.startFrame);
	totalDurationFrames_ = float(animationData_->railMetaData_.endFrame);
	JSON_OPEN("Resources/Data/Trolley/trolley.json");
	JSON_OBJECT("TrollerSpeed");
	JSON_LOAD_BY_NAME("maxTrollySpeed_", playbackSpeed_);
	JSON_CLOSE();
	JSON_OPEN("Resources/Data/RailAnimationPlayer/railAnimationPlayer.json");
	JSON_LOAD(level1GoalFrame_);
	JSON_LOAD(level2GoalFrame_);
	JSON_CLOSE();

	railTransform_.UpdateMatrix();
	convertRailTransform_ = RailSystem::RailConverter::ConvertToLeftHand(railTransform_);
	convertRailTransform_.UpdateMatrix();

	preCameraPosition_ = EvaluateRailPosition(0.0f);
}

void RailSystem::RailAnimationPlayer::Update(float deltaTime)
{
#ifdef DEBUG
	//止められているor終了していたら再生しない
	if (!isPlaying_ || IsFinished() || !animationData_ || animationData_->evalTimeKeys_.empty()) {
		return;
	}
#endif // _DEBUG



	if (isPlaying_) {
		//デルタタイムをフレームの進みに変換
		float frameIncrement = deltaTime * animationData_->railMetaData_.frameRate * playbackSpeed_;
		currentFrame_ += frameIncrement;

		auto result = AnimationUtils::CalculateCurrentTransform(
			animationData_->evalTimeKeys_,
			animationData_->railAnimation_.positionKeys,
			animationData_->railAnimation_.rotationKeys,
			currentFrame_
		);

		railTransform_ = result.first;
		convertRailTransform_ = result.second;

		if (deltaTime > 0.0001f) {
			float distance = (railTransform_.worldMatrix.GetTranslate() - preCameraPosition_).Length();
			realSpeed_ = distance / deltaTime;
		}
		else {
			realSpeed_ = 0.0f;
		}
		preCameraPosition_ = railTransform_.worldMatrix.GetTranslate();
	}
	else {
		auto result = AnimationUtils::CalculateCurrentTransform(
			animationData_->evalTimeKeys_,
			animationData_->railAnimation_.positionKeys,
			animationData_->railAnimation_.rotationKeys,
			currentFrame_
		);

		railTransform_ = result.first;
		convertRailTransform_ = result.second;

	}

#ifdef _DEBUG
	DrawImGui();
#endif // _DEBUG

}

void RailSystem::RailAnimationPlayer::Play()
{
	isPlaying_ = true;
}

void RailSystem::RailAnimationPlayer::Pause()
{
	isPlaying_ = false;
}

void RailSystem::RailAnimationPlayer::Stop()
{
	isPlaying_ = false;
	currentFrame_ = static_cast<float>(animationData_->railMetaData_.startFrame);

}

void RailSystem::RailAnimationPlayer::Loop()
{
	isPlaying_ = true;
	currentFrame_ = static_cast<float>(animationData_->railMetaData_.startFrame);
}

bool RailSystem::RailAnimationPlayer::IsFinished() const
{
	auto level = LevelManager::GetInstance()->GetLevel();
	switch (level)
	{
	case LevelManager::Level::LEVEL1:
	{
		if (currentFrame_ >= level1GoalFrame_) {
			return true;
		}
	}
	break;
	case LevelManager::Level::LEVEL2:
	{
		if (currentFrame_ >= level2GoalFrame_) {
			return true;
		}
	}
	break;
	default:
		break;
	}
	return false;
}

void RailSystem::RailAnimationPlayer::SetCurrentFrame(float frame)
{
	if (!animationData_) return;
	// フレーム番号をアニメーション範囲内にクランプする
	currentFrame_ = float((std::max)(float(animationData_->railMetaData_.startFrame), (std::min)(float(animationData_->railMetaData_.endFrame), frame)));
}

float RailSystem::RailAnimationPlayer::GetCurrentFrame() const
{
	return currentFrame_;
}

Vector3 RailSystem::RailAnimationPlayer::EvaluateRailPosition(float frame) const
{
	if (!animationData_) {
		return { 0.0f, 0.0f, 0.0f };
	}

	float evalTime = 0.0f;

	if (!animationData_->evalTimeKeys_.empty()) {
		const auto& keys = animationData_->evalTimeKeys_;

		if (frame <= keys.front().frame) {
			evalTime = keys.front().value;
		}
		else if (frame >= keys.back().frame) {
			evalTime = keys.back().value;
		}
		else {
			auto indices = FindKeyframeIndices(keys, frame);
			const auto& key1 = keys[indices.first];
			const auto& key2 = keys[indices.second];

			if (indices.first == indices.second) {
				evalTime = key1.value;
			}
			else {
				float frameDiff = key2.frame - key1.frame;
				float t = (std::abs(frameDiff) < 0.0001f) ? 0.0f : (frame - key1.frame) / frameDiff;
				t = std::max(0.0f, std::min(1.0f, t));

				evalTime = InterpolateScalar(key1, key2, t);
			}
		}
	}

	Vector3 resultPosition = { 0.0f, 0.0f, 0.0f };

	const auto& posKeys = animationData_->railAnimation_.positionKeys;
	if (!posKeys.empty()) {
		if (evalTime <= posKeys.front().frame) {
			resultPosition = posKeys.front().value;
		}
		else if (evalTime >= posKeys.back().frame) {
			resultPosition = posKeys.back().value;
		}
		else {
			auto posIndices = FindKeyframeIndices(posKeys, evalTime);
			const auto& posKey1 = posKeys[posIndices.first];
			const auto& posKey2 = posKeys[posIndices.second];

			float posFrameDiff = posKey2.frame - posKey1.frame;
			float posT = (std::abs(posFrameDiff) < 0.0001f) ? 0.0f : (evalTime - posKey1.frame) / posFrameDiff;
			posT = std::max(0.0f, std::min(1.0f, posT));

			resultPosition = InterpolatePosition(posKey1, posKey2, posT);
		}
	}


	return RailSystem::RailConverter::ConvertToLeftHand(resultPosition);
}
Quaternion RailSystem::RailAnimationPlayer::EvaluateRailRotation(float frame) const
{
	if (!animationData_) {
		return Quaternion::identity;
	}

	float evalTime = 0.0f;

	if (!animationData_->evalTimeKeys_.empty()) {
		const auto& keys = animationData_->evalTimeKeys_;

		if (frame <= keys.front().frame) {
			evalTime = keys.front().value;
		}
		else if (frame >= keys.back().frame) {
			evalTime = keys.back().value;
		}
		else {
			auto indices = FindKeyframeIndices(keys, frame);
			const auto& key1 = keys[indices.first];
			const auto& key2 = keys[indices.second];

			if (indices.first == indices.second) {
				evalTime = key1.value;
			}
			else {
				float frameDiff = key2.frame - key1.frame;
				float t = (std::abs(frameDiff) < 0.0001f) ? 0.0f : (frame - key1.frame) / frameDiff;
				t = std::max(0.0f, std::min(1.0f, t));

				evalTime = InterpolateScalar(key1, key2, t);
			}
		}
	}

	Quaternion resultRotation;

	const auto& rotKeys = animationData_->railAnimation_.rotationKeys;
	if (!rotKeys.empty()) {
		if (evalTime <= rotKeys.front().frame) {
			resultRotation = rotKeys.front().value;
		}
		else if (evalTime >= rotKeys.back().frame) {
			resultRotation = rotKeys.back().value;
		}
		else {
			auto rotIndices = FindKeyframeIndices(rotKeys, evalTime);
			const auto& rotKey1 = rotKeys[rotIndices.first];
			const auto& rotKey2 = rotKeys[rotIndices.second];

			float rotFrameDiff = rotKey2.frame - rotKey1.frame;
			float rotT = (std::abs(rotFrameDiff) < 0.0001f) ? 0.0f : (evalTime - rotKey1.frame) / rotFrameDiff;
			rotT = std::max(0.0f, std::min(1.0f, rotT));

			resultRotation = InterpolateRotation(rotKey1, rotKey2, rotT);
		}
	}
	return  RailSystem::RailConverter::ConvertToLeftHand(resultRotation);
}


Transform RailSystem::RailAnimationPlayer::EvaluateRailTransform(float frame) const
{
	Transform result;
	result.translate = EvaluateRailPosition(frame);
	result.rotate = EvaluateRailRotation(frame);
	result.UpdateMatrix();
	return result;
}

Vector3 RailSystem::RailAnimationPlayer::EvaluateLocalCameraPosition(float frame) const
{
	if (!animationData_) {
		return { 0.0f, 0.0f, 0.0f };
	}

	float evalTime = 0.0f;

	if (!animationData_->evalTimeKeys_.empty()) {
		const auto& keys = animationData_->evalTimeKeys_;

		if (frame <= keys.front().frame) {
			evalTime = keys.front().value;
		}
		else if (frame >= keys.back().frame) {
			evalTime = keys.back().value;
		}
		else {
			auto indices = FindKeyframeIndices(keys, frame);
			const auto& key1 = keys[indices.first];
			const auto& key2 = keys[indices.second];

			if (indices.first == indices.second) {
				evalTime = key1.value;
			}
			else {
				float frameDiff = key2.frame - key1.frame;
				float t = (std::abs(frameDiff) < 0.0001f) ? 0.0f : (frame - key1.frame) / frameDiff;
				t = std::max(0.0f, std::min(1.0f, t));

				evalTime = InterpolateScalar(key1, key2, t);
			}
		}
	}

	Vector3 resultPosition = { 0.0f, 0.0f, 0.0f };

	const auto& posKeys = animationData_->cameraAnimation_.positionKeys;
	if (!posKeys.empty()) {
		if (evalTime <= posKeys.front().frame) {
			resultPosition = posKeys.front().value;
		}
		else if (evalTime >= posKeys.back().frame) {
			resultPosition = posKeys.back().value;
		}
		else {
			auto posIndices = FindKeyframeIndices(posKeys, evalTime);
			const auto& posKey1 = posKeys[posIndices.first];
			const auto& posKey2 = posKeys[posIndices.second];

			float posFrameDiff = posKey2.frame - posKey1.frame;
			float posT = (std::abs(posFrameDiff) < 0.0001f) ? 0.0f : (evalTime - posKey1.frame) / posFrameDiff;
			posT = std::max(0.0f, std::min(1.0f, posT));

			resultPosition = InterpolatePosition(posKey1, posKey2, posT);
		}
	}


	return RailSystem::RailConverter::ConvertToLeftHand(resultPosition);
}
Quaternion RailSystem::RailAnimationPlayer::EvaluateLocalCameraRotation(float frame) const
{
	if (!animationData_) {
		return Quaternion::identity;
	}

	float evalTime = 0.0f;

	if (!animationData_->evalTimeKeys_.empty()) {
		const auto& keys = animationData_->evalTimeKeys_;

		if (frame <= keys.front().frame) {
			evalTime = keys.front().value;
		}
		else if (frame >= keys.back().frame) {
			evalTime = keys.back().value;
		}
		else {
			auto indices = FindKeyframeIndices(keys, frame);
			const auto& key1 = keys[indices.first];
			const auto& key2 = keys[indices.second];

			if (indices.first == indices.second) {
				evalTime = key1.value;
			}
			else {
				float frameDiff = key2.frame - key1.frame;
				float t = (std::abs(frameDiff) < 0.0001f) ? 0.0f : (frame - key1.frame) / frameDiff;
				t = std::max(0.0f, std::min(1.0f, t));

				evalTime = InterpolateScalar(key1, key2, t);
			}
		}
	}

	Quaternion resultRotation;

	const auto& rotKeys = animationData_->cameraAnimation_.rotationKeys;
	if (!rotKeys.empty()) {
		if (evalTime <= rotKeys.front().frame) {
			resultRotation = rotKeys.front().value;
		}
		else if (evalTime >= rotKeys.back().frame) {
			resultRotation = rotKeys.back().value;
		}
		else {
			auto rotIndices = FindKeyframeIndices(rotKeys, evalTime);
			const auto& rotKey1 = rotKeys[rotIndices.first];
			const auto& rotKey2 = rotKeys[rotIndices.second];

			float rotFrameDiff = rotKey2.frame - rotKey1.frame;
			float rotT = (std::abs(rotFrameDiff) < 0.0001f) ? 0.0f : (evalTime - rotKey1.frame) / rotFrameDiff;
			rotT = std::max(0.0f, std::min(1.0f, rotT));

			resultRotation = InterpolateRotation(rotKey1, rotKey2, rotT);
		}
	}
	return  RailSystem::RailConverter::ConvertCameraToLeftHand(resultRotation);
}


Transform RailSystem::RailAnimationPlayer::EvaluateLocalCameraTransform(float frame) const
{
	Transform result;
	result.translate = EvaluateLocalCameraPosition(frame);
	result.rotate = EvaluateLocalCameraRotation(frame);
	result.UpdateMatrix();
	return result;
}
Vector3 RailSystem::RailAnimationPlayer::EvaluateWorldCameraPosition(float frame) const
{
	Vector3 localPos = EvaluateLocalCameraPosition(frame);
	Vector3 worldPos = localPos * convertRailTransform_.worldMatrix;
	return worldPos;
}
Quaternion RailSystem::RailAnimationPlayer::EvaluateWorldCameraRotation(float frame) const
{
	Quaternion localRotation = EvaluateLocalCameraRotation(frame);
	Quaternion parentRotation = convertRailTransform_.worldMatrix.GetRotate();
	Quaternion worldRotation = parentRotation * localRotation;
	return worldRotation;
}
Transform RailSystem::RailAnimationPlayer::EvaluateWorldCameraTransform(float frame) const
{
	Transform t;
	t.translate = EvaluateWorldCameraPosition(frame);
	t.rotate = EvaluateLocalCameraRotation(frame);
	t.UpdateMatrix();
	return t;
}

#ifdef _DEBUG


void RailSystem::RailAnimationPlayer::DrawImGui()
{
	{
		ImGui::Begin("GameScene");
		if (ImGui::TreeNode("ゲームクリア制御（RailAnimationPlayer）")) {
			// 保存ボタン
			if (ImGui::Button("Save", ImVec2(-1.0f, 0.0f))) {
				JSON_OPEN("Resources/Data/RailAnimationPlayer/railAnimationPlayer.json");
				JSON_SAVE(level1GoalFrame_);
				JSON_SAVE(level2GoalFrame_);
				JSON_CLOSE();
			}

			ImGui::Separator();

			ImGui::DragFloat("Level1ゴールのフレーム", &level1GoalFrame_);
			ImGui::DragFloat("Level2ゴールのフレーム", &level2GoalFrame_);

			ImGui::TreePop();
		}
		ImGui::End();

	}
}

#endif // _DEBUG