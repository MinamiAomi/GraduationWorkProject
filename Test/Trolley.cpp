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

	currentCharge_ = maxNormalCharge_ * 0.5f;


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
	transform_.rotate = bankRotation * shakeRotation_;
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
#ifdef _DEBUG
	if (!isDebugTrollySpeed_) {
#endif // _DEBUG
		if (trollyState_ != State::Nitro && trollyState_ != State::Burst) {

			if (isHitFlashlight_) {
				currentCharge_ += accelerationRate_ * deltaTime * 60.0f;
			}
			else {
				currentCharge_ -= decelerationRate_ * deltaTime * 60.0f;
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

			//シェイク
			float timeRate = stateTimer_ / burstDuration_;
			float decay = std::lerp(1.0f, 0.0f, std::clamp(timeRate, 0.0f, 1.0f));

			float maxShakeAngle = 2.0f * Math::ToRadian;
			float currentShake = maxShakeAngle * decay;

			Vector3 noiseEuler = {
			rnd_.NextFloatRange(-1.0f, 1.0f) * currentShake,
			rnd_.NextFloatRange(-1.0f, 1.0f) * currentShake,
			rnd_.NextFloatRange(-1.0f, 1.0f) * currentShake
			};

			shakeRotation_ = Quaternion::MakeFromEulerAngle(noiseEuler);
			// バースト復帰判定
			if (stateTimer_ >= burstDuration_) {
				RecoverFromBurst();
				shakeRotation_ = Quaternion::identity;
			}
		}
		break;
		default:
			break;
		}
#ifdef _DEBUG
	}
#endif // _DEBUG
}

void Trolley::UpdateBanking(float deltaTime)
{
	float currentFrame = railCameraAnimationPlayer_->GetCurrentFrame();

	Vector3 posNow = railCameraAnimationPlayer_->EvaluatePosition(currentFrame);
	Vector3 posFuture = railCameraAnimationPlayer_->EvaluatePosition(currentFrame + lookAheadForBank_);

	Vector3 diff = posFuture - posNow;

	float targetBankAngle = 0.0f;

	if (diff.LengthSquare() > 1e-5f) {

		Vector3 dirToFuture = diff.Normalized();

		Vector3 forwardNow = transform_.worldMatrix.GetRotate() * Vector3(0, 0, 1);

		Quaternion blenderRotation = railCameraAnimationPlayer_->EvaluateRotation(currentFrame);
		Vector3 railUpVector = blenderRotation * Vector3(0, 1, 0);

		Vector3 curveCross = Vector3::Cross(forwardNow, dirToFuture);

		float turnIntensity = Vector3::Dot(curveCross, railUpVector);

		targetBankAngle = -turnIntensity * railCameraAnimationPlayer_->GetRealSpeed() * bankingAmount_;

		targetBankAngle = std::clamp(targetBankAngle, -45.0f * Math::ToRadian, 45.0f * Math::ToRadian);
	}

	if (std::isfinite(targetBankAngle)) {
		currentBankAngle_ = std::lerp(currentBankAngle_, targetBankAngle, deltaTime * bankingSmoothTime_);
	}

	if (!std::isfinite(currentBankAngle_)) {
		currentBankAngle_ = 0.0f;
	}
}

void Trolley::OnNormalState()
{
	trollyState_ = State::Normal;
	stateTimer_ = 0.0f;
	nitroAccumulateTimer_ = 0.0f;
}

void Trolley::OnOverchargeState()
{
	trollyState_ = Trolley::State::Overcharge;
	stateTimer_ = 0.0f;
	nitroAccumulateTimer_ = 0.0f;
}

void Trolley::OnNitroState()
{
	trollyState_ = State::Nitro;
	stateTimer_ = 0.0f;
	nitroAccumulateTimer_ = 0.0f;

}

void Trolley::RecoverFromNitro()
{
	trollyState_ = State::Normal;
	currentCharge_ = batteryAfterNitro_;
	stateTimer_ = 0.0f;
	nitroAccumulateTimer_ = 0.0f;
}

void Trolley::RecoverFromBurst()
{
	trollyState_ = State::Normal;
	stateTimer_ = 0.0f;
	nitroAccumulateTimer_ = 0.0f;
}

void Trolley::OnBurstState()
{
	trollyState_ = State::Burst;
	stateTimer_ = 0.0f;
	currentCharge_ = batteryAfterBurst_;
	nitroAccumulateTimer_ = 0.0f;
}

#ifdef _DEBUG
void Trolley::DrawImGui() {
	// =================================================================================
	// 1. ステータス表示ウィンドウ (リアルタイム監視用)
	//    プレイ中ずっと出しておきたい情報は視認性重視
	// =================================================================================
	ImGui::Begin("Trolley Status");

	// --- 現在の状態 (英語のまま) ---
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
	// STATEは見出しなので英語のまま強調
	ImGui::TextColored(stateColor, "STATE: %s", stateStr);

	// --- 速度表示 ---
	// 単位や意味がわかるように日本語を添える
	ImGui::SliderFloat("現在のトロッコ速度 (Speed)", &currentSpeed_, 0.0f, nitroSpeed_, "%.2f km/h");

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
	// 数値だけだと何かわからないので日本語追加
	sprintf_s(buf, "充電量: %.1f", currentCharge_);
	ImGui::ProgressBar(batteryRatio, ImVec2(-1.0f, 0.0f), buf);
	ImGui::PopStyleColor();

	// --- ニトロ蓄積ゲージ (Overcharge時のみ表示) ---
	if (trollyState_ == State::Overcharge && currentCharge_ >= nitroThreshold_) {
		float nitroRatio = nitroAccumulateTimer_ / nitroChargeTime_;

		ImGui::TextColored(ImVec4(0, 1, 1, 1), "Nitro Readying..."); // 雰囲気重視で英語
		ImGui::SameLine();
		ImGui::ProgressBar(nitroRatio, ImVec2(-1.0f, 0.0f), "HOLD!"); // ゲーム的な指示は英語で
	}

	// --- デバッグ用スイッチ ---
	ImGui::Separator();
	// 変数名そのままだとプランナーが触りづらいので機能名にする
	ImGui::Checkbox("デバッグ：自然減速を停止", &isDebugTrollySpeed_);

	ImGui::End();


	// =================================================================================
	// 2. パラメータ調整ウィンドウ (GameSceneタブ内)
	//    レベルデザイン調整用。意味を誤解しないよう詳細な日本語にする。
	// =================================================================================
	ImGui::Begin("GameScene");

	if (ImGui::TreeNode("トロッコ制御 (Trolley)")) {
		// 保存ボタン
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

		ImGui::Separator();

		if (ImGui::TreeNode("基本座標 (Transform)")) {
			ImGui::DragFloat3("本体オフセット", &trolleyOffset_.x, 0.01f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("当たり判定 (Collision)")) {
			ImGui::DragFloat3("判定オフセット", &batteryOffset_.x, 0.01f);
			ImGui::DragFloat("判定半径 (Radius)", &batteryRadius_, 0.01f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("パラメータ設定 (Parameters)")) {

			if (ImGui::TreeNode("速度・加速度 (Speed & Accel)")) {
				ImGui::DragFloat("通常時の最高速度", &maxSpeed_, 0.01f);
				ImGui::DragFloat("ニトロ時の最高速度", &nitroSpeed_, 0.01f);
				ImGui::DragFloat("バースト時の最高速度", &burstSpeed_, 0.01f);
				ImGui::Spacing();
				ImGui::DragFloat("光を当てた時の加速量", &accelerationRate_, 0.1f);
				ImGui::DragFloat("自然減速量", &decelerationRate_, 0.1f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("しきい値・バッテリー容量 (Thresholds)")) {
				// スライダーの説明を具体的に
				ImGui::DragFloat("通常上限 (100%)", &maxNormalCharge_, 1.0f, 0.0f, nitroThreshold_);
				ImGui::DragFloat("ニトロ発動ライン", &nitroThreshold_, 1.0f, maxNormalCharge_, burstThreshold_);
				ImGui::DragFloat("バースト発生ライン", &burstThreshold_, 1.0f, nitroThreshold_, 300.0f);

				ImGui::Spacing();
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "イベント終了後の残量");
				ImGui::DragFloat("ニトロ終了後", &batteryAfterNitro_, 1.0f);
				ImGui::DragFloat("バースト終了後", &batteryAfterBurst_, 1.0f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("時間制御 (Timings)")) {
				ImGui::DragFloat("ニトロ発動に必要な維持時間", &nitroChargeTime_, 0.1f, 0.0f, 10.0f, "%.1f秒");
				ImGui::DragFloat("ニトロ持続時間", &nitroDuration_, 0.1f, 0.0f, 10.0f, "%.1f秒");
				ImGui::DragFloat("バースト演出時間", &burstDuration_, 0.1f, 0.0f, 10.0f, "%.1f秒");
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("バンキング (傾き演出)")) {
			ImGui::DragFloat("傾きの強さ (Amount)", &bankingAmount_, 0.1f);
			ImGui::DragFloat("追従速度 (Smooth)", &bankingSmoothTime_, 0.01f);
			ImGui::DragFloat("カーブの先読み距離", &lookAheadForBank_, 0.01f);
			ImGui::TreePop();
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
