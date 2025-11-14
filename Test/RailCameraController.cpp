#include "RailCameraController.h"

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <iostream> 
#ifdef _DEBUG
#include <iomanip>
#include <sstream>
#include <Windows.h>
#endif // _DEBUG

RailCameraSystem::RailCameraController::RailCameraController(std::shared_ptr<const RailCameraSystem::RailCameraAnimation> animationData)
{
	if (!animationData) {
		throw std::invalid_argument("AnimationData cannot be null.");
	}
	animationData_ = animationData;
	isPlaying_ = false;
	playbackSpeed_ = 1.0f;
	//メタデータからアニメーションの開始と終了の位置を取得
	currentFrame_ = float(animationData_->railCameraMetaData_.startFrame);
	totalDurationFrames_ = float(animationData_->railCameraMetaData_.endFrame);
}

void RailCameraSystem::RailCameraController::Update(float deltaTime)
{
	//止められているor終了していたら再生しない
	if (!isPlaying_ || IsFinished() || !animationData_ || animationData_->evalTimeKeys_.empty()) {
		return;
	}

	//デルタタイムをフレームの進みに変換
	float frameIncrement = deltaTime * animationData_->railCameraMetaData_.frameRate * playbackSpeed_;
	currentFrame_ += frameIncrement;

	//アニメーションの終了フレームを超えないようにクランプ
	if (currentFrame_ > totalDurationFrames_) {
		currentFrame_ = totalDurationFrames_;
		//いったんループ
		Loop();
	}
#ifdef _DEBUG
	{
		std::wostringstream woss;
		woss << L"Current Frame: " << std::fixed << std::setprecision(2) << currentFrame_ << L"\n";
		OutputDebugStringW(woss.str().c_str());
	}
#endif // _DEBUG

}

Transform RailCameraSystem::RailCameraController::GetCurrentTransform() const
{

	Transform transform;

	if (!animationData_ || animationData_->positionKeys_.empty() || animationData_->rotationKeys_.empty()) {
		return transform;
	}

	float evalTime = GetCurrentEvalTime();



	// 2. 取得したevalTimeを「仮想的なフレーム」として、位置と回転のキーを検索
	// 位置キーフレーム
	const auto& posKeys = animationData_->positionKeys_;
	if (!posKeys.empty()) {
		if (evalTime <= posKeys.front().frame) {
			transform.translate = posKeys.front().value;
			{ std::wostringstream woss; woss << L"  Pos: Using first key.\n"; OutputDebugStringW(woss.str().c_str()); }
		}
		else if (evalTime >= posKeys.back().frame) {
			transform.translate = posKeys.back().value;
			{ std::wostringstream woss; woss << L"  Pos: Using last key.\n"; OutputDebugStringW(woss.str().c_str()); }
		}
		else {
			auto posIndices = FindKeyframeIndices(posKeys, evalTime);
			{ std::wostringstream woss; woss << L"  Pos Indices: (" << posIndices.first << L", " << posIndices.second << L")\n"; OutputDebugStringW(woss.str().c_str()); }
			const auto& posKey1 = posKeys[posIndices.first];
			const auto& posKey2 = posKeys[posIndices.second];
			{ std::wostringstream woss; woss << L"  Pos Key1 Frame: " << posKey1.frame << L", Pos Key2 Frame: " << posKey2.frame << L"\n"; OutputDebugStringW(woss.str().c_str()); }
			float posFrameDiff = posKey2.frame - posKey1.frame;
			float posT = (std::abs(posFrameDiff) < 0.0001f) ? 0.0f : (evalTime - posKey1.frame) / posFrameDiff;
			posT = std::max(0.0f, std::min(1.0f, posT)); // クランプ
			{ std::wostringstream woss; woss << L"  Pos FrameDiff: " << posFrameDiff << L", posT: " << std::fixed << std::setprecision(4) << posT << L"\n"; OutputDebugStringW(woss.str().c_str()); }
			transform.translate = InterpolatePosition(posKey1, posKey2, posT);
		}
	}


	// 回転キーフレーム
	const auto& rotKeys = animationData_->rotationKeys_;
	if (!rotKeys.empty()) {
		if (evalTime <= rotKeys.front().frame) {
			transform.rotate = rotKeys.front().value;
#ifdef _DEBUG
			{ std::wostringstream woss; woss << L"  Rot: Using first key.\n"; OutputDebugStringW(woss.str().c_str()); }
#endif // _DEBUG
		}
		else if (evalTime >= rotKeys.back().frame) {
			transform.rotate = rotKeys.back().value;
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
			transform.rotate = InterpolateRotation(rotKey1, rotKey2, rotT);
#ifdef _DEBUG
			{ std::wostringstream woss; woss << L"  Rot Indices: (" << rotIndices.first << L", " << rotIndices.second << L")\n"; OutputDebugStringW(woss.str().c_str()); }
			{ std::wostringstream woss; woss << L"  Rot Key1 Frame: " << rotKey1.frame << L", Rot Key2 Frame: " << rotKey2.frame << L"\n"; OutputDebugStringW(woss.str().c_str()); }
			{ std::wostringstream woss; woss << L"  Rot FrameDiff: " << rotFrameDiff << L", rotT: " << std::fixed << std::setprecision(4) << rotT << L"\n"; OutputDebugStringW(woss.str().c_str()); }
#endif // _DEBUG

		}
	}
#ifdef _DEBUG

	{
		std::wostringstream woss;
		woss << L"--- GetCurrentTransform --- EvalTime: " << std::fixed << std::setprecision(4) << evalTime << L"\n";
		OutputDebugStringW(woss.str().c_str());
	}
	{
		std::wostringstream woss;
		woss << L"  Final Position: (" << transform.translate.x << L", " << transform.translate.y << L", " << transform.translate.z << L")\n";
		OutputDebugStringW(woss.str().c_str());
	}
	{
		std::wostringstream woss;
		woss << L"  Final Rotation: (w:" << transform.rotate.w << L", x:" << transform.rotate.x << L", y:" << transform.rotate.y << L", z:" << transform.rotate.z << L")\n";
		OutputDebugStringW(woss.str().c_str());
#endif // _DEBUG
	}

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

void RailCameraSystem::RailCameraController::SetCurrentFrame(int frame)
{
	if (!animationData_) return;
	// フレーム番号をアニメーション範囲内にクランプする
	currentFrame_ = float((std::max)(animationData_->railCameraMetaData_.startFrame, (std::min)(animationData_->railCameraMetaData_.endFrame, frame)));
}

float RailCameraSystem::RailCameraController::GetCurrentFrame() const
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

template<typename T>
inline std::pair<size_t, size_t> RailCameraSystem::RailCameraController::FindKeyframeIndices(const std::vector<T>& keys, float currentFrame) const
{
	if (keys.empty()) return { 0, 0 };

	// frame以下の最大のキーを探すイテレータを取得
	// std::lower_bound は [first, last) で f より小さい最初の要素を返す
	// ここでは frame "以下" の最後のキーを見つけたい
	auto it = std::lower_bound(keys.begin(), keys.end(), currentFrame,
		[](const T& key, float f) {
			// key.frame < f だと frame と同じ値のキーが次の区間に入るため
			return key.frame < f;
		});

	// イテレータが最初を指す場合 (frameが最初のキーより前か同じ)
	if (it == keys.begin()) {
		return { 0, 0 };
	}

	// イテレータが最後を指す場合 (frameが最後のキーより後)
	if (it == keys.end()) {
		return { keys.size() - 1, keys.size() - 1 };
	}

	// it は frame より大きい最初のキーを指しているので、
	// prevIndex は it の一つ前
	size_t nextIndex = std::distance(keys.begin(), it);
	size_t prevIndex = nextIndex - 1;

	// frameがキーフレームと完全に一致する場合、同じインデックスを返す
	// (補間係数t=0またはt=1になるようにするため)
	if (std::abs(keys[prevIndex].frame - currentFrame) < 0.0001f) {
		return { prevIndex, prevIndex };
	}
	if (std::abs(keys[nextIndex].frame - currentFrame) < 0.0001f) {
		return { nextIndex, nextIndex };
	}


	return { prevIndex, nextIndex };
}

float RailCameraSystem::RailCameraController::GetCurrentEvalTime() const
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

float RailCameraSystem::RailCameraController::InterpolateScalar(const RailCameraSystem::ScalarKeyframe& key1, const RailCameraSystem::ScalarKeyframe& key2, float currentFrame) const
{
	if (currentFrame <= 0.0f) return key1.value;
	if (currentFrame >= 1.0f) return key2.value;

	// BEZIER補間の場合
	if (key1.interpolation == "BEZIER") {

		// 制御点を定義 (X=frame, Y=value)
		Vector2 p0 = { key1.frame, key1.value };
		Vector2 p1 = { key1.handle.right.x, key1.handle.right.y };
		Vector2 p2 = { key2.handle.left.x, key2.handle.left.y };
		Vector2 p3 = { key2.frame, key2.value };

		// X軸(時間)の正規化された位置(t_norm)から、
		//    ベジェ曲線のパラメータt (t_b)を逆算する
		float t_b = FindBezierTForX(currentFrame, p0, p1, p2, p3);

		// 逆算した t_b を使って、Y軸(値)を計算する
		return EvaluateBezier(t_b, p0, p1, p2, p3).y;
	}

	// "LINEAR", "CONSTANT", または不明な場合は線形補間
	return std::lerp(key1.value, key2.value, currentFrame);
}

Vector3 RailCameraSystem::RailCameraController::InterpolatePosition(const RailCameraSystem::PositionKeyframe& key1, const RailCameraSystem::PositionKeyframe& key2, float currentFrame) const
{
	if (currentFrame <= 0.0f) return key1.value;
	if (currentFrame >= 1.0f) return key2.value;

	return Vector3::Lerp(currentFrame, key1.value, key2.value);
}

Quaternion RailCameraSystem::RailCameraController::InterpolateRotation(const RailCameraSystem::RotationKeyframe& key1, const RailCameraSystem::RotationKeyframe& key2, float currentFrame) const
{
	if (currentFrame <= 0.0f) return key1.value;
	if (currentFrame >= 1.0f) return key2.value;
	return Quaternion::Slerp(currentFrame, key1.value, key2.value);
}

float RailCameraSystem::RailCameraController::FindBezierTForX(float targetX, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3) const
{
	// X軸を0-1に正規化するための準備
	float frameRange = p3.x - p0.x;
	// 垂直なカーブ（ありえないが）の場合は0を返す
	if (std::abs(frameRange) < 0.0001f) {
		return 0.0f;
	}

	// X軸だけを0-1の範囲に正規化した制御点を作成
	// (Yはここでは使わないので0でもよい)
	Vector2 pp0 = { 0.0f, 0.0f };
	Vector2 pp1 = { (p1.x - p0.x) / frameRange, 0.0f };
	Vector2 pp2 = { (p2.x - p0.x) / frameRange, 0.0f };
	Vector2 pp3 = { 1.0f, 0.0f };

	// 二分探索でx_targetに最も近いxを持つパラメータtを探す
	float t_low = 0.0f;
	float t_high = 1.0f;
	float t_guess = targetX; // 最初の推測値はx_target自身

	for (int i = 0; i < 8; ++i) { // 8回もあれば十分な精度
		float x_guess = EvaluateBezier(t_guess, pp0, pp1, pp2, pp3).x;
		float error = x_guess - targetX;

		if (std::abs(error) < 0.001f) {
			break; // 許容誤差
		}

		if (error < 0.0f) { // 推測が小さすぎた
			t_low = t_guess;
		}
		else { // 推測が大きすぎた
			t_high = t_guess;
		}
		t_guess = (t_high + t_low) * 0.5f;
	}

	return t_guess;
}

Vector2 RailCameraSystem::RailCameraController::EvaluateBezier(float t, const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3) const
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
