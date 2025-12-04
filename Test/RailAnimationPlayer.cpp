#include "RailAnimationPlayer.h"

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <iostream> 

#include "Engine/File/JsonConverter.h"
#include "RailConverter.h"
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
	currentFrame_ = float(animationData_->railCameraMetaData_.startFrame);
	totalDurationFrames_ = float(animationData_->railCameraMetaData_.endFrame);
	JSON_OPEN("Resources/Data/Trolley/trolley.json");
	JSON_OBJECT("TrollerSpeed");
	JSON_LOAD_BY_NAME("maxTrollySpeed_", playbackSpeed_);
	JSON_CLOSE();
}

void RailSystem::RailAnimationPlayer::Update(float deltaTime)
{
#ifdef DEBUG
	//止められているor終了していたら再生しない
	if (!isPlaying_ || IsFinished() || !animationData_ || animationData_->evalTimeKeys_.empty()) {
		return;
	}
#endif // _DEBUG



#ifdef _DEBUG
	if (isPlaying_) {
#endif // _DEBUG
		//デルタタイムをフレームの進みに変換
		float frameIncrement = deltaTime * animationData_->railCameraMetaData_.frameRate * playbackSpeed_;
		currentFrame_ += frameIncrement;

		CalculateCurrentTransform();
#ifdef _DEBUG
	}
	else {
		CalculateCurrentTransform();
	}
#endif // _DEBUG

#ifdef _DEBUG
	{
		std::wostringstream woss;
		woss << L"Current Frame: " << std::fixed << std::setprecision(2) << currentFrame_ << L"\n";
		OutputDebugStringW(woss.str().c_str());

		ImGui::Begin("GameScene");

		if (ImGui::TreeNode("RailAnimationPlayer")) {

			if (isPlaying_) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: Playing >>");
			}
			else {
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Status: Paused ||");
			}

			//区切り線
			ImGui::Separator();

			float minFrame = static_cast<float>(animationData_->railCameraMetaData_.startFrame);
			float maxFrame = static_cast<float>(animationData_->railCameraMetaData_.endFrame);

			// スライダーで直感的に位置を変更・確認できるようにする
			ImGui::Text("Timeline");
			ImGui::SliderFloat("##FrameSlider", &currentFrame_, minFrame, maxFrame, "Frame: %.2f");
			ImGui::InputFloat("##Frame", &currentFrame_);
			currentFrame_ = std::clamp(currentFrame_, float(animationData_->railCameraMetaData_.startFrame), float(animationData_->railCameraMetaData_.endFrame));

			// 進捗バー
			float progress = (currentFrame_ - minFrame) / (maxFrame - minFrame);
			ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));

			// 余白
			ImGui::Spacing();


			// 再生ボタン
			if (ImGui::Button("Play")) {
				Play();
			}
			//横並びに
			ImGui::SameLine();

			// 一時停止ボタン
			if (ImGui::Button("Pause")) {
				Pause();
			}
			//横並びに
			ImGui::SameLine();

			// 停止
			if (ImGui::Button("Stop")) {
				Stop();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("MetaDeta")) {
				ImGui::Text("Start Frame : %d", animationData_->railCameraMetaData_.startFrame);
				ImGui::Text("End Frame   : %d", animationData_->railCameraMetaData_.endFrame);

				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}
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
	currentFrame_ = static_cast<float>(animationData_->railCameraMetaData_.startFrame);

}

void RailSystem::RailAnimationPlayer::Loop()
{
	isPlaying_ = true;
	currentFrame_ = static_cast<float>(animationData_->railCameraMetaData_.startFrame);
}

void RailSystem::RailAnimationPlayer::SetCurrentFrame(int frame)
{
	if (!animationData_) return;
	// フレーム番号をアニメーション範囲内にクランプする
	currentFrame_ = float((std::max)(animationData_->railCameraMetaData_.startFrame, (std::min)(animationData_->railCameraMetaData_.endFrame, frame)));
}

float RailSystem::RailAnimationPlayer::GetCurrentFrame() const
{
	if (!animationData_ || animationData_->evalTimeKeys_.empty()) {
		return 0.0f;
	}
	const auto& keys = animationData_->evalTimeKeys_;

	// アニメーション範囲外の処理
	if (currentFrame_ <= keys.front().frame) return keys.front().value;
	if (currentFrame_ >= keys.back().frame) return keys.back().value;



	auto indices = FindKeyframeIndices(keys, currentFrame_);


	const auto& key1 = keys[indices.first];
	const auto& key2 = keys[indices.second];


	// 同じインデックス -> キーフレーム上にいる
	if (indices.first == indices.second) {
		return key1.value;
	}

	// 補間係数 t_norm を計算
	float frameDiff = key2.frame - key1.frame;
	// ゼロ除算を避ける
	float t_norm = (std::abs(frameDiff) < 0.0001f) ? 0.0f : (currentFrame_ - key1.frame) / frameDiff;

	// t_norm を 0.0 - 1.0 の範囲にクランプ
	t_norm = std::max(0.0f, std::min(1.0f, t_norm));


	// スカラー値の補間を実行
	float result = InterpolateScalar(key1, key2, t_norm);


#ifdef _DEBUG
	{
		std::wostringstream woss;
		woss << L"--- GetCurrentEvalTime --- CurrentFrame: " << std::fixed << std::setprecision(4) << currentFrame_ << L"\n";
		OutputDebugStringW(woss.str().c_str());
	}
	{
		std::wostringstream woss;
		woss << L"  Found Indices: (" << indices.first << L", " << indices.second << L")\n";
		OutputDebugStringW(woss.str().c_str());
	}

	{
		std::wostringstream woss;
		woss << L"  Key1 Frame: " << key1.frame << L", Value: " << key1.value
			<< L" | Key2 Frame: " << key2.frame << L", Value: " << key2.value << L"\n";
		OutputDebugStringW(woss.str().c_str());
	}

	{
		std::wostringstream woss;
		woss << L"  On Keyframe. Returning value: " << key1.value << L"\n";
		OutputDebugStringW(woss.str().c_str());
	}
	{
		std::wostringstream woss;
		woss << L"  FrameDiff: " << frameDiff << L", t_norm: " << t_norm << L"\n";
		OutputDebugStringW(woss.str().c_str());
	}
	{
		std::wostringstream woss;
		woss << L"  Interpolated EvalTime: " << result << L"\n";
		OutputDebugStringW(woss.str().c_str());
	}
#endif // _DEBUG

	return result;
}

Vector3 RailSystem::RailAnimationPlayer::EvaluatePosition(float frame) const
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

	const auto& posKeys = animationData_->positionKeys_;
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
Quaternion RailSystem::RailAnimationPlayer::EvaluateRotation(float frame) const
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

	const auto& rotKeys = animationData_->rotationKeys_;
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
void RailSystem::RailAnimationPlayer::CalculateCurrentTransform()
{
	if (!animationData_ || animationData_->positionKeys_.empty() || animationData_->rotationKeys_.empty()) {
		return;
	}

	float evalTime = GetCurrentEvalTime();

	// 位置キーフレーム
	const auto& posKeys = animationData_->positionKeys_;
	if (!posKeys.empty()) {
		if (evalTime <= posKeys.front().frame) {
			transform_.translate = posKeys.front().value;
#ifdef _DEBUG
			{ std::wostringstream woss; woss << L"  Pos: Using first key.\n"; OutputDebugStringW(woss.str().c_str()); }
#endif // _DEBUG
		}
		else if (evalTime >= posKeys.back().frame) {
			transform_.translate = posKeys.back().value;
#ifdef _DEBUG
			{ std::wostringstream woss; woss << L"  Pos: Using last key.\n"; OutputDebugStringW(woss.str().c_str()); }
#endif // _DEBUG
		}
		else {
			auto posIndices = FindKeyframeIndices(posKeys, evalTime);
#ifdef _DEBUG
			{ std::wostringstream woss; woss << L"  Pos Indices: (" << posIndices.first << L", " << posIndices.second << L")\n"; OutputDebugStringW(woss.str().c_str()); }
#endif // _DEBUG
			const auto& posKey1 = posKeys[posIndices.first];
			const auto& posKey2 = posKeys[posIndices.second];
#ifdef _DEBUG
			{ std::wostringstream woss; woss << L"  Pos Key1 Frame: " << posKey1.frame << L", Pos Key2 Frame: " << posKey2.frame << L"\n"; OutputDebugStringW(woss.str().c_str()); }
#endif // _DEBUG
			float posFrameDiff = posKey2.frame - posKey1.frame;
			float posT = (std::abs(posFrameDiff) < 0.0001f) ? 0.0f : (evalTime - posKey1.frame) / posFrameDiff;
			posT = std::max(0.0f, std::min(1.0f, posT)); // クランプ
#ifdef _DEBUG
			{ std::wostringstream woss; woss << L"  Pos FrameDiff: " << posFrameDiff << L", posT: " << std::fixed << std::setprecision(4) << posT << L"\n"; OutputDebugStringW(woss.str().c_str()); }
#endif // _DEBUG
			transform_.translate = InterpolatePosition(posKey1, posKey2, posT);
		}
	}
	// 回転キーフレーム
	const auto& rotKeys = animationData_->rotationKeys_;
	if (!rotKeys.empty()) {
		if (evalTime <= rotKeys.front().frame) {
			transform_.rotate = rotKeys.front().value;
#ifdef _DEBUG
			{ std::wostringstream woss; woss << L"  Rot: Using first key.\n"; OutputDebugStringW(woss.str().c_str()); }
#endif // _DEBUG
		}
		else if (evalTime >= rotKeys.back().frame) {
			transform_.rotate = rotKeys.back().value;
#ifdef _DEBUG
			{ std::wostringstream woss; woss << L"  Rot: Using last key.\n"; OutputDebugStringW(woss.str().c_str()); }
#endif // _DEBUG
		}
		else {
			auto rotIndices = FindKeyframeIndices(rotKeys, evalTime);
			const auto& rotKey1 = rotKeys[rotIndices.first];
			const auto& rotKey2 = rotKeys[rotIndices.second];
			float rotFrameDiff = rotKey2.frame - rotKey1.frame;
			float rotT = (std::abs(rotFrameDiff) < 0.0001f) ? 0.0f : (evalTime - rotKey1.frame) / rotFrameDiff;
			rotT = std::max(0.0f, std::min(1.0f, rotT));
			transform_.rotate = InterpolateRotation(rotKey1, rotKey2, rotT);
#ifdef _DEBUG
			{ std::wostringstream woss; woss << L"  Rot Indices: (" << rotIndices.first << L", " << rotIndices.second << L")\n"; OutputDebugStringW(woss.str().c_str()); }
			{ std::wostringstream woss; woss << L"  Rot Key1 Frame: " << rotKey1.frame << L", Rot Key2 Frame: " << rotKey2.frame << L"\n"; OutputDebugStringW(woss.str().c_str()); }
			{ std::wostringstream woss; woss << L"  Rot FrameDiff: " << rotFrameDiff << L", rotT: " << std::fixed << std::setprecision(4) << rotT << L"\n"; OutputDebugStringW(woss.str().c_str()); }
#endif // _DEBUG

		}
	}

	transform_.UpdateMatrix();
	convertTransform_ = RailSystem::RailConverter::ConvertToLeftHand(transform_);
	convertTransform_.UpdateMatrix();
#ifdef _DEBUG
	{
		std::wostringstream woss;
		woss << L"--- GetCurrentTransform --- EvalTime: " << std::fixed << std::setprecision(4) << evalTime << L"\n";
		OutputDebugStringW(woss.str().c_str());
	}
	{
		std::wostringstream woss;
		woss << L"  Final Position: (" << convertTransform_.translate.x << L", " << convertTransform_.translate.y << L", " << convertTransform_.translate.z << L")\n";
		OutputDebugStringW(woss.str().c_str());
	}
	{
		std::wostringstream woss;
		woss << L"  Final Rotation: (w:" << convertTransform_.rotate.w << L", x:" << convertTransform_.rotate.x << L", y:" << convertTransform_.rotate.y << L", z:" << convertTransform_.rotate.z << L")\n";
		OutputDebugStringW(woss.str().c_str());
	}
#endif // _DEBUG
}

template<typename T>
inline std::pair<size_t, size_t> RailSystem::RailAnimationPlayer::FindKeyframeIndices(const std::vector<T>& keys, float currentFrame) const
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

float RailSystem::RailAnimationPlayer::GetCurrentEvalTime() const
{
	if (!animationData_ || animationData_->evalTimeKeys_.empty()) {
		return 0.0f;
	}
	const auto& keys = animationData_->evalTimeKeys_;
	auto indices = FindKeyframeIndices(keys, currentFrame_);

	const auto& key1 = keys[indices.first];
	const auto& key2 = keys[indices.second];

	if (indices.first == indices.second) {
		return key1.value;
	}

	float t = (currentFrame_ - key1.frame) / (key2.frame - key1.frame);
	return InterpolateScalar(key1, key2, std::max(0.0f, std::min(1.0f, t)));
}

float RailSystem::RailAnimationPlayer::InterpolateScalar(const RailSystem::ScalarKeyframe& key1, const RailSystem::ScalarKeyframe& key2, float currentFrame) const
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

Vector3 RailSystem::RailAnimationPlayer::InterpolatePosition(const RailSystem::PositionKeyframe& key1, const RailSystem::PositionKeyframe& key2, float currentFrame) const
{
	if (currentFrame <= 0.0f) return key1.value;
	if (currentFrame >= 1.0f) return key2.value;

	return Vector3::Lerp(currentFrame, key1.value, key2.value);
}

Quaternion RailSystem::RailAnimationPlayer::InterpolateRotation(const RailSystem::RotationKeyframe& key1, const RailSystem::RotationKeyframe& key2, float currentFrame) const
{
	if (currentFrame <= 0.0f) return key1.value;
	if (currentFrame >= 1.0f) return key2.value;
	return Quaternion::Slerp(currentFrame, key1.value, key2.value);
}

float RailSystem::RailAnimationPlayer::FindBezierTForX(float targetX, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3) const
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

Vector2 RailSystem::RailAnimationPlayer::EvaluateBezier(float t, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3) const
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
