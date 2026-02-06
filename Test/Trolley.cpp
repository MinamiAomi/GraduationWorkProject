#include "Trolley.h"

#include "Framework/AssetManager.h"

#include "Engine/File/JsonConverter.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG


Trolley::Trolley()
{
	model_.SetModel(AssetManager::GetInstance()->modelMap.Get("trolley")->Get());

	for (auto& collider : batteryColliders_) {
		collider = std::make_shared<SphereCollider>(
			CollisionCategory::PLAYER,
			(CollisionCategory::FLASHLIGHT | CollisionCategory::GIMMICKTRIGGER),
			Vector3::zero,
			0.0f
		);
	}

	//teilLight_ = std::make_shared<SpotLight>();

	//RenderManager::GetInstance()->GetLightManager().Add(teilLight_);

	//teilLightTransform_.SetParent(&transform_);

	trolleyUI_.SetTrolley(this);

}
void Trolley::Initialize()
{
	JSON_OPEN("Resources/Data/Trolley/trolley.json");
	JSON_OBJECT("TrollerSpeed");
	JSON_LOAD(startFrame_);
	JSON_LOAD(maxSpeed_);
	JSON_LOAD(minSpeed_);
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
	for (uint8_t i = 0; i < BatteryNum; i++) {
		std::string key = "batteryOffset:" + std::to_string(i);
		JSON_LOAD_BY_NAME(key.c_str(), batteryOffsets_.at(i));
	}
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

	currentCharge_ = 0.0f;


	nitroAccumulateTimer_ = 0.0f;

	stateTimer_ = 0.0f;

	isHitFlashlight_ = false;


	for (uint8_t i = 0; i < BatteryNum; i++) {
		batteryTransforms_.at(i).translate = batteryOffsets_.at(i);
		//batteryTransforms_.at(i).SetParent(&transform_);
		batteryTransforms_.at(i).UpdateMatrix();
		batteryColliders_.at(i)->center = batteryTransforms_.at(i).worldMatrix.GetTranslate();
		batteryColliders_.at(i)->radius = batteryRadius_;
	}

	//teilOffset_ = { 0.0f,5.0f,-2.0f };
	//teilLight_->position = teilLightTransform_.worldMatrix.GetTranslate() + teilOffset_;
	//teilLight_->direction = teilLightTransform_.worldMatrix.GetForward();
	//teilLight_->color = Color(Vector4(0.7f, 0.65f, 0.2f, 1.0f));
	//teilLight_->intensity = 3.0f;
	//teilLight_->range = 25.0f;
	//teilLight_->angle = 45.0f * Math::ToRadian;
	//teilLight_->falloffStartAngle = 15.0f * Math::ToRadian;
	//teilLight_->decay = 1.0f;

	shakeRotation_ = Quaternion::identity;
	shakeOffset_ = Vector3::zero;

	trolleyUI_.Initialize(transform_);
}

void Trolley::Update(float deltaTime)
{

	UpdateCollision();
	UpdateState(deltaTime);
	UpdateBanking(deltaTime);

	Quaternion bankRotation = Quaternion::MakeFromAngleAxis(currentBankAngle_, Vector3(0.0f, 0.0f, 1.0f));

	transform_.translate = trolleyOffset_ + shakeOffset_;
	transform_.rotate = bankRotation * shakeRotation_;
	transform_.UpdateMatrix();
	//teilLightTransform_.translate = teilOffset_;
	//teilLightTransform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);

	for (uint8_t i = 0; i < BatteryNum; i++) {
		batteryTransforms_.at(i).translate = batteryOffsets_.at(i);
		batteryTransforms_.at(i).UpdateMatrix();
		batteryColliders_.at(i)->center = batteryTransforms_.at(i).worldMatrix.GetTranslate();
		batteryColliders_.at(i)->radius = batteryRadius_;
	}


	//teilLight_->position = teilLightTransform_.worldMatrix.GetTranslate();
	//teilLight_->direction = -teilLightTransform_.worldMatrix.GetForward();


	trolleyUI_.Update();

#ifdef _DEBUG
	DrawImGui();
	//teilLight_->DrawImGui("teilLight_");
	//ImGui::DragFloat3("teilOffset_", &teilOffset_.x, 0.1f);
#endif // _DEBUG

}

void Trolley::UpdateState(float deltaTime)
{
#ifdef _DEBUG
	if (!isDebugTrollySpeed_) {
#endif // _DEBUG
		if (trollyState_ != State::Nitro && trollyState_ != State::Burst) {

			if (isHitFlashlight_) {
				currentCharge_ += accelerationRate_ * deltaTime * 60.0f * centerRate_;
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
			if (startFrame_ <= railCameraAnimationPlayer_->GetCurrentFrame()) {
				currentSpeed_ = std::lerp(minSpeed_, maxSpeed_, currentCharge_ / maxNormalCharge_);
			}
			else {
				currentSpeed_ = std::lerp(0.0f, maxSpeed_, currentCharge_ / maxNormalCharge_);
			}
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

			float timeRate = stateTimer_ / burstDuration_;
			float decay = std::lerp(1.0f, 0.0f, std::clamp(timeRate, 0.0f, 1.0f));


			float shakeX = std::sin(stateTimer_ * 50.0f) * 0.8f * Math::ToRadian;
			float shakeY = std::sin(stateTimer_ * 43.0f) * 0.1f * Math::ToRadian;
			float shakeZ = std::sin(stateTimer_ * 60.0f) * 1.5f * Math::ToRadian;

			Vector3 noiseEuler = {
				shakeX * decay,
				shakeY * decay,
				shakeZ * decay
			};

			shakeRotation_ = Quaternion::MakeFromEulerAngle(noiseEuler);

			float posShakeAmount = 0.1f * decay;

			shakeOffset_ = {
				rnd_.NextFloatRange(-0.2f, 0.2f) * posShakeAmount,
				rnd_.NextFloatRange(0.0f, 1.0f) * posShakeAmount,
				0.0f
			};

			// バースト復帰判定
			if (stateTimer_ >= burstDuration_) {
				RecoverFromBurst();

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

	Vector3 posNow = railCameraAnimationPlayer_->EvaluateRailPosition(currentFrame);
	Vector3 posFuture = railCameraAnimationPlayer_->EvaluateRailPosition(currentFrame + lookAheadForBank_);

	Vector3 diff = posFuture - posNow;

	float targetBankAngle = 0.0f;

	if (diff.LengthSquare() > 1e-5f) {

		Vector3 dirToFuture = diff.Normalized();

		Vector3 forwardNow = transform_.worldMatrix.GetRotate() * Vector3(0, 0, 1);

		Quaternion blenderRotation = railCameraAnimationPlayer_->EvaluateRailRotation(currentFrame);
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

	shakeRotation_ = Quaternion::identity;
	shakeOffset_ = Vector3::zero;
}

void Trolley::OnBurstState()
{
	trollyState_ = State::Burst;
	stateTimer_ = 0.0f;
	currentCharge_ = batteryAfterBurst_;
	nitroAccumulateTimer_ = 0.0f;
}

float Trolley::CalculateCenterRate(const Vector3& center, float radius) {
	const auto collider = flashlight_->GetCollider();

	Vector3 conePos = collider->center;
	Quaternion coneRot = collider->quaternion;

	Vector3 spherePos = center;
	float sphereRadius = radius;

	Vector3 diff = spherePos - conePos;
	Quaternion invRot = coneRot.Conjugate();
	Vector3 localPos = invRot * diff;

	float coneH = collider->height;
	float coneR = collider->radius;

	float axisY = localPos.y;
	float axisR = std::sqrt(localPos.x * localPos.x + localPos.z * localPos.z);

	float clampedY = std::clamp(axisY, 0.0f, coneH);
	float maxRadiusAtHeight = (coneH - clampedY) * (coneR / coneH);

	float allowedDistance = maxRadiusAtHeight + sphereRadius;

	if (allowedDistance <= 0.001f) {
		return 1.0f;
	}

	float score = 1.0f - (axisR / allowedDistance);

	// 底上げ（+0.1fはお好みで）してClamp
	return std::clamp(score + 0.1f, 0.0f, 1.0f);
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
	ImGui::SliderFloat("バッテリーを照らすライトの真ん中具合", &centerRate_, 0.0f, 1.0f, "%.2f");

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
			JSON_SAVE(startFrame_);
			JSON_SAVE(maxSpeed_);
			JSON_SAVE(minSpeed_);
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
			for (uint8_t i = 0; i < BatteryNum; i++) {
				std::string key = "batteryOffset:" + std::to_string(i);
				JSON_SAVE_BY_NAME(key.c_str(), batteryOffsets_.at(i));
			}
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
			for (uint8_t i = 0; i < BatteryNum; i++) {
				std::string key = "判定オフセット" + std::to_string(i) + ":";
				ImGui::DragFloat3(key.c_str(), &batteryOffsets_.at(i).x, 0.01f);
			}
			ImGui::DragFloat("判定半径 (Radius)", &batteryRadius_, 0.01f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("パラメータ設定 (Parameters)")) {

			ImGui::DragFloat("のろのろ進み始めるフレーム", &startFrame_, 0.01f);
			if (ImGui::TreeNode("速度・加速度 (Speed & Accel)")) {
				ImGui::DragFloat("通常時の最高速度", &maxSpeed_, 0.01f);
				ImGui::DragFloat("通常時の最低速度", &minSpeed_, 0.01f);
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
	centerRate_ = 0.0f;

	for (auto& collider : batteryColliders_) {
		if (!collider->GetCollidedWith().empty()) {
			for (const auto& collidedWith : collider->GetCollidedWith()) {
				switch (collidedWith->categoryBits)
				{
				case CollisionCategory::NONE:
					break;
				case CollisionCategory::PLAYER:
					break;
				case CollisionCategory::FLASHLIGHT:
				{
					isHitFlashlight_ = true;

					centerRate_ = CalculateCenterRate(collider->center, collider->radius);
				}
				break;
				case CollisionCategory::LIGHT:
					break;
				case CollisionCategory::ENEMY:
					break;
				case CollisionCategory::ALL:
					break;
				default:
					break;
				}

			}
		}
	}
}
