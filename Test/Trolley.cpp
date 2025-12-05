#include "Trolley.h"

#include "Framework/AssetManager.h"

#include "Engine/File/JsonConverter.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG


Trolley::Trolley()
{
	model_.SetModel(AssetManager::GetInstance()->modelMap.Get("trolley")->Get());
	batteryCollider_ = std::make_shared<SphereCollider>(
		CollisionCategory::PLAYER,
		(CollisionCategory::FLASHLIGHT),
		Vector3::zero,
		0.0f
	);
}
void Trolley::Initialize()
{
	JSON_OPEN("Resources/Data/Trolley/trolley.json");
	JSON_OBJECT("TrollerSpeed");
	JSON_LOAD(maxSpeed_);
	JSON_LOAD(burstSpeed_);
	JSON_LOAD(nitroSpeed_);
	JSON_LOAD(accelerationRate_);
	JSON_LOAD(decelerationRate_);
	JSON_LOAD(maxNormalCharge_);
	JSON_LOAD(nitroThreshold_);
	JSON_LOAD(burstThreshold_);
	JSON_LOAD(nitroChargeTime_);
	JSON_LOAD(nitroDuration_);
	JSON_LOAD(burstDuration_);
	JSON_LOAD(batteryAfterNitro_);
	JSON_LOAD(batteryAfterBurst_);
	JSON_ROOT();
	JSON_OBJECT("Trolley");
	JSON_LOAD(trolleyOffset_);
	JSON_ROOT();
	JSON_OBJECT("Battery");
	JSON_LOAD(batteryOffset_);
	JSON_LOAD(batteryRadius_);
	JSON_ROOT();
	JSON_OBJECT("Banking");
	JSON_LOAD(bankingAmount_);
	JSON_LOAD(bankingSmoothTime_);
	JSON_LOAD(lookAheadForBank_);
	JSON_ROOT();
	JSON_CLOSE();

	transform_.translate = trolleyOffset_;
	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);
	trollyState_ = State::Normal;
	currentSpeed_ = 0.0f;

	currentCharge_ = 100.0f;


	nitroAccumulateTimer_ = 0.0f;

	stateTimer_ = 0.0f;

	isHitFlashlight_ = false;

	batteryTransform_.translate = batteryOffset_;
	batteryTransform_.SetParent(&transform_);
	batteryTransform_.UpdateMatrix();

	batteryCollider_->center = batteryTransform_.worldMatrix.GetTranslate();
	batteryCollider_->radius = batteryRadius_;

	trolleyUI_.Initialize(transform_);
	trolleyUI_.SetTrolley(this);
}

void Trolley::Update(float deltaTime)
{

	UpdateCollision();
	UpdateState(deltaTime);
	UpdateBanking(deltaTime);

	Quaternion bankRotation = Quaternion::MakeFromAngleAxis(currentBankAngle_, Vector3(0.0f, 0.0f, 1.0f));

	transform_.translate = trolleyOffset_;
	transform_.rotate = bankRotation;
	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);

	batteryTransform_.translate = batteryOffset_;
	batteryTransform_.UpdateMatrix();
	batteryCollider_->center = batteryTransform_.worldMatrix.GetTranslate();
	batteryCollider_->radius = batteryRadius_;

	trolleyUI_.Update();

#ifdef _DEBUG
	DrawImGui();
#endif // _DEBUG

}

void Trolley::UpdateState(float deltaTime)
{
	if (trollyState_ != State::Nitro && trollyState_ != State::Burst) {

		if (isHitFlashlight_) {
			currentCharge_ += accelerationRate_ * deltaTime * 60.0f;
		}
		else {
#ifdef _DEBUG
			if (!isDebugTrollySpeed_)
#endif
			{
				currentCharge_ -= decelerationRate_ * deltaTime * 60.0f;
			}
		}
	}

	currentCharge_ = std::clamp(currentCharge_, 0.0f, burstThreshold_);

	switch (trollyState_)
	{
	case Trolley::State::Normal:
	{
		currentSpeed_ = std::lerp(0.0f, maxSpeed_, currentCharge_ / maxNormalCharge_);
		//チャージが満タンならオーバーチャージに移行
		if (currentCharge_ > maxNormalCharge_) {
			OnOverchargeState();
		}
	}
	break;
	case Trolley::State::Overcharge:
	{
		currentSpeed_ = maxSpeed_;

		//バースト判定
		if (currentCharge_ >= burstThreshold_) {
			OnBurstState();
			return;
		}

		//ノーマル判定
		if (currentCharge_ <= maxNormalCharge_) {
			OnNormalState();
			return;
		}

		//ニトロ判定
		if (currentCharge_ >= nitroThreshold_) {
			nitroAccumulateTimer_ += deltaTime;
			// 規定時間維持できたら発動
			if (nitroAccumulateTimer_ >= nitroChargeTime_) {
				OnNitroState();
			}
		}
		else {
			nitroAccumulateTimer_ -= deltaTime;
			nitroAccumulateTimer_ = std::max(nitroAccumulateTimer_, 0.0f);
		}
	}
	break;
	case Trolley::State::Nitro:
	{
		currentSpeed_ = nitroSpeed_;

		stateTimer_ += deltaTime;
		// ニトロ終了判定
		if (stateTimer_ >= nitroDuration_) {
			RecoverFromNitro();
		}
	}
	break;
	case Trolley::State::Burst:
	{
		currentSpeed_ = burstSpeed_;

		stateTimer_ += deltaTime;
		// バースト復帰判定
		if (stateTimer_ >= burstDuration_) {
			RecoverFromBurst();
		}
	}
	break;
	default:
		break;
	}
}

void Trolley::UpdateBanking(float deltaTime)
{
	float currentFrame = railCameraAnimationPlayer_->GetCurrentFrame();

	Vector3 posNow = railCameraAnimationPlayer_->EvaluatePosition(currentFrame);
	Vector3 posFuture = railCameraAnimationPlayer_->EvaluatePosition(currentFrame + lookAheadForBank_);

	Vector3 forwardNow = transform_.worldMatrix.GetRotate() * Vector3(0, 0, 1);

	Quaternion blenderRotation = railCameraAnimationPlayer_->EvaluateRotation(currentFrame);
	Vector3 railUpVector = blenderRotation * Vector3(0, 1, 0);

	Vector3 dirToFuture = (posFuture - posNow).Normalized();

	Vector3 curveCross = Vector3::Cross(forwardNow, dirToFuture);

	float turnIntensity = Vector3::Dot(curveCross, railUpVector);

	float targetBankAngle = -turnIntensity * railCameraAnimationPlayer_->GetRealSpeed() * bankingAmount_;

	targetBankAngle = std::clamp(targetBankAngle, -45.0f * Math::ToRadian, 45.0f * Math::ToRadian);

	currentBankAngle_ = std::lerp(currentBankAngle_, targetBankAngle, deltaTime * bankingSmoothTime_);
}

void Trolley::OnNormalState()
{
	trollyState_ = State::Normal;
	stateTimer_ = 0.0f;
}

void Trolley::OnOverchargeState()
{
	trollyState_ = Trolley::State::Overcharge;
	nitroAccumulateTimer_ = 0.0f;
}

void Trolley::OnNitroState()
{
	trollyState_ = State::Nitro;
	stateTimer_ = 0.0f;
}

void Trolley::RecoverFromNitro()
{
	trollyState_ = State::Normal;
	currentCharge_ = batteryAfterNitro_;
	stateTimer_ = 0.0f;
}

void Trolley::RecoverFromBurst()
{
	trollyState_ = State::Normal;
	stateTimer_ = 0.0f;
}

void Trolley::OnBurstState()
{
	trollyState_ = State::Burst;
	stateTimer_ = 0.0f;
	currentCharge_ = batteryAfterBurst_;
}
#ifdef _DEBUG
void Trolley::DrawImGui() {
	// =================================================================================
	// 1. ステータス表示ウィンドウ (リアルタイム監視用)
	//    プレイ中ずっと出しておきたい情報はここ
	// =================================================================================
	ImGui::Begin("Trolley Statusあいうえおアイウエオ亜伊宇江御");

	// --- 現在の状態 (色付きテキスト) ---
	const char* stateStr = "";
	ImVec4 stateColor = ImVec4(1, 1, 1, 1);
	switch (trollyState_) {
	case State::Normal:
		stateStr = "NORMAL";
		stateColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f); // 緑
		break;
	case State::Overcharge:
		stateStr = "OVERCHARGE";
		stateColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // 黄色
		break;
	case State::Nitro:
		stateStr = "NITRO";
		stateColor = ImVec4(0.0f, 1.0f, 1.0f, 1.0f); // 水色
		break;
	case State::Burst:
		stateStr = "BURST";
		stateColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // 赤
		break;
	}
	ImGui::TextColored(stateColor, "STATE: %s", stateStr);

	// --- 速度表示 ---
	ImGui::SliderFloat("Speed", &currentSpeed_, 0.0f, nitroSpeed_, "km/h: %.2f");

	ImGui::Separator();

	// --- バッテリーゲージ ---
	// 全体(0~BurstThreshold)に対する割合
	float batteryRatio = currentCharge_ / burstThreshold_;

	// ゲージの色分け
	if (trollyState_ == State::Burst)
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // 赤
	else if (currentCharge_ >= nitroThreshold_)
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.5f, 0.0f, 1.0f)); // オレンジ
	else
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // 緑

	char buf[32];
	sprintf_s(buf, "Charge: %.1f", currentCharge_);
	ImGui::ProgressBar(batteryRatio, ImVec2(-1.0f, 0.0f), buf);
	ImGui::PopStyleColor();

	// --- ニトロ蓄積ゲージ (Overcharge時のみ表示) ---
	if (trollyState_ == State::Overcharge) {
		float nitroRatio = nitroAccumulateTimer_ / nitroChargeTime_;
		ImGui::TextColored(ImVec4(0, 1, 1, 1), "Nitro Casting...");
		ImGui::SameLine();
		ImGui::ProgressBar(nitroRatio, ImVec2(-1.0f, 0.0f), "HOLD!");
	}

	// --- デバッグ用スイッチ ---
	ImGui::Separator();
	ImGui::Checkbox("isDebugTrollySpeed", &isDebugTrollySpeed_);

	ImGui::End();


	// =================================================================================
	// 2. パラメータ調整ウィンドウ (GameSceneタブ内)
	//    レベルデザイン調整用。JSON保存機能付き。
	// =================================================================================
	ImGui::Begin("GameScene");

	if (ImGui::TreeNode("Troller")) {
		if (ImGui::TreeNode("Offset")) {
			ImGui::DragFloat3("Trolley Offset", &trolleyOffset_.x, 0.01f);
			ImGui::DragFloat3("Battery Offset", &batteryOffset_.x, 0.01f);
			ImGui::DragFloat("Battery Radius", &batteryRadius_, 0.01f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Speed")) {
			// パラメータをグループ分けして表示
			if (ImGui::TreeNode("Speed & Accel")) {
				ImGui::DragFloat("Max Speed", &maxSpeed_, 0.01f);
				ImGui::DragFloat("Nitro Speed", &nitroSpeed_, 0.01f);
				ImGui::DragFloat("Burst Speed (Slow)", &burstSpeed_, 0.01f);
				ImGui::DragFloat("Accel Rate", &accelerationRate_, 0.1f);
				ImGui::DragFloat("Decel Rate", &decelerationRate_, 0.1f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Battery Thresholds")) {
				ImGui::DragFloat("Normal Max (100%)", &maxNormalCharge_, 1.0f, 0.0f, nitroThreshold_);
				ImGui::DragFloat("Nitro Threshold", &nitroThreshold_, 1.0f, maxNormalCharge_, burstThreshold_);
				ImGui::DragFloat("Burst Threshold", &burstThreshold_, 1.0f, nitroThreshold_, 300.0f);

				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Result Values:");
				ImGui::DragFloat("After Nitro Charge", &batteryAfterNitro_, 1.0f);
				ImGui::DragFloat("After Burst Charge", &batteryAfterBurst_, 1.0f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Timings")) {
				ImGui::DragFloat("Nitro Hold Time (sec)", &nitroChargeTime_, 0.1f);
				ImGui::DragFloat("Nitro Duration (sec)", &nitroDuration_, 0.1f);
				ImGui::DragFloat("Burst Duration (sec)", &burstDuration_, 0.1f);
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Banking")) {
			ImGui::DragFloat("Banking Amount", &bankingAmount_, 0.1f);
			ImGui::DragFloat("Smooth Time", &bankingSmoothTime_, 0.01f);
			ImGui::DragFloat("Look Ahead", &lookAheadForBank_, 0.01f);
			ImGui::Separator();

			ImGui::TreePop();
		}
		ImGui::Separator();

		// 保存ボタンを一番上に配置
		if (ImGui::Button("Save", ImVec2(-1.0f, 0.0f))) {
			JSON_OPEN("Resources/Data/Trolley/trolley.json");

			JSON_OBJECT("TrollerSpeed");
			JSON_SAVE(maxSpeed_);
			JSON_SAVE(burstSpeed_);
			JSON_SAVE(nitroSpeed_);
			JSON_SAVE(accelerationRate_);
			JSON_SAVE(decelerationRate_);
			JSON_SAVE(maxNormalCharge_);
			JSON_SAVE(nitroThreshold_);
			JSON_SAVE(burstThreshold_);
			JSON_SAVE(nitroChargeTime_);
			JSON_SAVE(nitroDuration_);
			JSON_SAVE(burstDuration_);
			JSON_SAVE(batteryAfterNitro_);
			JSON_SAVE(batteryAfterBurst_);
			JSON_ROOT();
			JSON_OBJECT("Trolley");
			JSON_SAVE(trolleyOffset_);
			JSON_ROOT();
			JSON_OBJECT("Battery");
			JSON_SAVE(batteryOffset_);
			JSON_SAVE(batteryRadius_);
			JSON_ROOT();
			JSON_OBJECT("Banking");
			JSON_SAVE(bankingAmount_);
			JSON_SAVE(bankingSmoothTime_);
			JSON_SAVE(lookAheadForBank_);
			JSON_ROOT();
			JSON_CLOSE();
		}

		ImGui::TreePop(); // Trolley Controller
	}

	ImGui::End();
}
#endif
void Trolley::UpdateCollision()
{
	isHitFlashlight_ = false;
	if (!batteryCollider_->GetCollidedWith().empty()) {
		for (const auto& collider : batteryCollider_->GetCollidedWith()) {
			if (collider->categoryBits == CollisionCategory::FLASHLIGHT) {
				//フラッシュライトが点灯しているか
				if (flashlight_->GetIsLighting()) {
					isHitFlashlight_ = true;
					break;
				}
			}
		}
	}
}
