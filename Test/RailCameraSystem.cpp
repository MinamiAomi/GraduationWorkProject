#include "RailCameraSystem.h"

#include "Engine/File/JsonConverter.h"
#include "RailConverter.h"

#include "Engine/Input/Input.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

void RailSystem::RailCameraSystem::Initialize()
{
	JSON_OPEN("Resources/Data/RailCamera/railCameraSystem.json");
	JSON_LOAD(cameraOffset_);
	JSON_LOAD(pointOfGazeOffset_);
	JSON_OBJECT("Fov");
	JSON_LOAD(baseFov_);
	JSON_LOAD(maxFov_);
	JSON_LOAD(fovLerpSpeed_);
	JSON_LOAD(referenceMaxSpeed_);
	JSON_ROOT();
	JSON_OBJECT("LookAhead");
	JSON_LOAD(futureFrame_);
	JSON_ROOT();
	JSON_CLOSE();

	Reset();
}

void RailSystem::RailCameraSystem::Reset()
{
	currentFov_ = baseFov_;

	transform_.translate = cameraOffset_;
	transform_.UpdateMatrix();

	UpdateFov(1.0f / 60.0f);
	UpdateLookAhead(1.0f / 60.0f);

	currentLookRotation_ = Quaternion::identity;
}

void RailSystem::RailCameraSystem::Update(float deltaTime)
{
	UpdateFov(deltaTime);
	UpdateLookAhead(deltaTime);

	float currentFrame = railCameraAnimationPlayer_->GetCurrentFrame();

	Vector3 localPos = railCameraAnimationPlayer_->EvaluateCameraPosition(currentFrame);
	Quaternion localRotation = railCameraAnimationPlayer_->EvaluateCameraRotation(currentFrame);

	transform_.translate = localPos + cameraOffset_;

	if (transform_.GetParent()) {
		transform_.rotate = currentLookRotation_ * localRotation;
	}
	else {
		transform_.rotate = currentLookRotation_;
	}

	transform_.UpdateMatrix();

#ifdef _DEBUG
	DrawImGui();
#endif // _DEBUG

}

void RailSystem::RailCameraSystem::UpdateFov(float deltaTime)
{

	deltaTime;
	float speedRatio = std::clamp(railCameraAnimationPlayer_->GetRealSpeed() / referenceMaxSpeed_, 0.0f, 1.0f);

	float t = speedRatio * speedRatio;

	float targetFov = Math::Lerp(t, baseFov_, maxFov_);

	float interpolationFactor = std::clamp(fovLerpSpeed_ * (1.0f / 60.0f), 0.0f, 1.0f);

	currentFov_ = std::lerp(currentFov_, targetFov, interpolationFactor);

}
void RailSystem::RailCameraSystem::UpdateLookAhead(float deltaTime)
{
	transform_.UpdateMatrix();
	Vector3 currentPos = transform_.translate;
	float currentFrame = railCameraAnimationPlayer_->GetCurrentFrame();
	float futureFrame = currentFrame + futureFrame_;
	Vector3 targetPos = railCameraAnimationPlayer_->EvaluateCameraPosition(futureFrame) + pointOfGazeOffset_;


	Vector3 forwardVector = Vector3::forward;
	if ((targetPos - currentPos).LengthSquare() > 1e-05f) {
		forwardVector = (targetPos - currentPos).Normalized();
	}

	Vector3 upVector = Vector3::up;

	if (transform_.GetParent()) {
		Quaternion parentRotation = transform_.GetParent()->worldMatrix.GetRotate();
		upVector = parentRotation * Vector3::up;
	}


	Quaternion targetRotation = Quaternion::MakeLookRotation(forwardVector, upVector);

	float t = std::clamp(deltaTime * 5.0f, 0.0f, 1.0f);
	currentLookRotation_ = Quaternion::Slerp(t, currentLookRotation_, targetRotation);
}


#ifdef _DEBUG
void RailSystem::RailCameraSystem::DrawImGui()
{
	// GameSceneウィンドウ内への描画
	ImGui::Begin("GameScene");

	if (ImGui::TreeNode("レールカメラ制御 (RailCamera)")) {

		// =========================================================
		// 1. 一括保存ボタン (最上部に配置)
		// =========================================================
		// 目立つように幅いっぱいに配置
		if (ImGui::Button("Save", ImVec2(-1.0f, 0.0f))) {
			JSON_OPEN("Resources/Data/RailCamera/railCameraSystem.json");

			// --- Root階層 (Offset) ---
			// 元のJSON構造を維持するためにRootで保存
			JSON_SAVE(cameraOffset_);
			JSON_SAVE(pointOfGazeOffset_);

			// --- Fov階層 ---
			JSON_OBJECT("Fov");
			JSON_SAVE(baseFov_);
			JSON_SAVE(maxFov_);
			JSON_SAVE(fovLerpSpeed_);
			JSON_SAVE(referenceMaxSpeed_);
			JSON_ROOT(); // Object終了

			// --- LookAhead階層 ---
			JSON_OBJECT("LookAhead");
			JSON_SAVE(futureFrame_);
			JSON_ROOT(); // Object終了

			JSON_CLOSE();
		}

		ImGui::Separator();
		ImGui::Spacing();

		// =========================================================
		// 2. カメラ位置 (Offset)
		// =========================================================
		if (ImGui::TreeNode("基本位置 (Offset)")) {
			// トロッコなどの対象物からどれだけずらすか
			ImGui::DragFloat3("追従オフセット", &cameraOffset_.x, 0.1f, -50.0f, 50.0f);
			ImGui::DragFloat3("注視点オフセット（元のレールからどのくらい離れたところを見るか）", &pointOfGazeOffset_.x, 0.1f, -50.0f, 50.0f);
			ImGui::TreePop();
		}

		// =========================================================
		// 3. 速度演出 (FOV Effect)
		//    速度に応じて視野角を変える演出の設定
		// =========================================================
		if (ImGui::TreeNode("速度演出・視野角 (FOV & Speed)")) {

			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "レールアニメーションの現実速度を参照");
			ImGui::Text("MaxRef: %.2f km/h", referenceMaxSpeed_); // 単位は仮定ですがあるとわかりやすい
			ImGui::Separator();

			// ラジアン <-> 度数法 変換
			float baseFovDeg = baseFov_ * Math::ToDegree;
			float maxFovDeg = maxFov_ * Math::ToDegree;

			// 調整項目
			ImGui::DragFloat("基本視野角 (Base FOV)", &baseFovDeg, 0.1f, 10.0f, 120.0f, "%.1f deg");
			ImGui::DragFloat("最大視野角 (Max FOV)", &maxFovDeg, 0.1f, baseFovDeg, 150.0f, "%.1f deg");

			ImGui::Spacing();
			ImGui::DragFloat("演出が最大になる速度", &referenceMaxSpeed_, 1.0f, 0.0f, 300.0f);
			ImGui::DragFloat("視野角の変化速度 (Lerp)", &fovLerpSpeed_, 0.01f, 0.0f, 1.0f);

			// 値を戻す
			baseFov_ = baseFovDeg * Math::ToRadian;
			maxFov_ = maxFovDeg * Math::ToRadian;

			ImGui::TreePop();
		}

		// =========================================================
		// 4. 注視点予測 (LookAhead)
		// =========================================================
		if (ImGui::TreeNode("注視点制御 (LookAt)")) {
			// カメラがレールの「どれくらい先」を見るか
			ImGui::DragFloat("先読みフレーム数 (Prediction)", &futureFrame_, 0.1f, 0.0f, 120.0f, "%.1f frames");
			ImGui::TreePop();
		}

		ImGui::TreePop(); // RailCamera
	}
	ImGui::End();
}

#endif // _DEBUG