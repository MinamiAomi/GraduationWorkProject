#include "Trolley.h"

#include "Framework/AssetManager.h"

#include "Engine/File/JsonConverter.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

Trolley* Trolley::singletonInstance = nullptr;

void Trolley::CreateInstance() {
    if (singletonInstance != nullptr) {
        delete singletonInstance;
        singletonInstance = nullptr;
    }
    singletonInstance = new Trolley;
}

void Trolley::DestroyInstance() {
    if (singletonInstance != nullptr) {
        delete singletonInstance;
        singletonInstance = nullptr;
    }
}

Trolley::Trolley()
{
    auto assetManager = AssetManager::GetInstance();

	for (auto& collider : batteryColliders_) {
		collider = std::make_shared<SphereCollider>(
			CollisionCategory::TROLLEYBATTERY,
			(CollisionCategory::ENEMY | CollisionCategory::FLASHLIGHT | CollisionCategory::GIMMICKTRIGGER | CollisionCategory::OBSTACLE),
			Vector3::zero,
			0.0f
		);
	}

	trolleyCollider_ = std::make_shared<CapsuleCollider>(
		CollisionCategory::TROLLEY,
		(CollisionCategory::ENEMY | CollisionCategory::GIMMICKTRIGGER | CollisionCategory::OBSTACLE),
		Vector3::zero,
		0.0f,
		0.0f,
		Quaternion::identity
	);


	normalSESource_ = assetManager->soundMap.Get("SE_TROLLY_NORMAL")->Get();
	nitroSESource_ = assetManager->soundMap.Get("SE_TROLLY_NITRO")->Get();
	burstSESource_ = assetManager->soundMap.Get("SE_TROLLY_BURST")->Get();
	crashSESource_ = assetManager->soundMap.Get("SE_TROLLY_CRASH")->Get();
	for (uint32_t i = 0; i < kNitroBoostSECount; i++) {
		std::string soundName = "SE_TROLLY_NITRO_BOOST" + std::to_string(i);
		nitroBoostSESources_[i] = assetManager->soundMap.Get(soundName)->Get();
	}
	nitroFizzSESource_ = assetManager->soundMap.Get("SE_TROLLY_NITRO_FIZZ")->Get();
	chargeSESource_ = assetManager->soundMap.Get("SE_TROLLY_CHARGE")->Get();

    //teilLight_ = std::make_shared<SpotLight>();

    //RenderManager::GetInstance()->GetLightManager().Add(teilLight_);

    //teilLightTransform_.SetParent(&transform_);

    model_.SetModel(assetManager->modelMap.Get("trolley")->Get());

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
	JSON_LOAD(nitroChargeTime_);
	JSON_LOAD(nitroDuration_);
	JSON_LOAD(batteryAfterNitro_);
	JSON_LOAD(batDecrease_);
	JSON_LOAD(nitroAccelerationRate_);
	JSON_LOAD(nitroDecelerationRate_);
	JSON_ROOT();
	JSON_OBJECT("TrolleyCollider");
	JSON_LOAD(trolleyColliderOffset_);
	JSON_LOAD(trolleyColliderHeight_);
	JSON_LOAD(trolleyColliderRadius_);
	JSON_LOAD(trolleyColliderQuaternion_);
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
		batteryTransforms_.at(i).SetParent(nullptr);
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

    batsNum_ = 0;

	trolleyColliderTransform_.SetParent(&transform_);
	trolleyColliderTransform_.UpdateMatrix();



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

	trolleyColliderTransform_.translate = trolleyColliderOffset_;
	trolleyColliderTransform_.rotate = trolleyColliderQuaternion_;

	trolleyColliderTransform_.UpdateMatrix();

	trolleyCollider_->center = trolleyColliderTransform_.worldMatrix.GetTranslate();
	trolleyCollider_->radius = trolleyColliderRadius_;
	trolleyCollider_->quaternion = trolleyColliderTransform_.worldMatrix.GetRotate();
	trolleyCollider_->height = trolleyColliderHeight_;

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

    // oto
    UpdateSound();
    trolleyUI_.Update();

#ifdef _DEBUG
    DrawImGui();
    //teilLight_->DrawImGui("teilLight_");
    //ImGui::DragFloat3("teilOffset_", &teilOffset_.x, 0.1f);
#endif // _DEBUG

}

void Trolley::UpdateState(float deltaTime)
{
    if (!isPause_) {
        //通常時のバッテリーチャージ
        if (trollyState_ != State::Nitro && trollyState_ != State::Stop) {

			//バッテリーが十分でライトが当たっているとき
			//Batsの数で減らす
			currentCharge_ -= batsNum_ * batDecrease_;
			if (ghostDamage_ > 0.0f) {
				currentCharge_ -= ghostDamage_;
				ghostDamage_ = 0.0f;
			}
			if (isHitFlashlight_ &&
				!flashlight_->GetBatteryRemaining()) {

                currentCharge_ += accelerationRate_ * deltaTime * 60.0f * centerRate_;

                if (!chargeSESource_.IsPlaying()) {
                    chargeSESource_.Play(false);
                    auto t = std::sin((centerRate_ * Math::Pi) * 0.5f);
                    chargeSESource_.SetVolume(t);
                    chargeSESource_.SetPitch(0.7f + t * 0.3f);
                }
            }

            //バッテリーが無い状態でライトが当たっているとき
            else if (isHitFlashlight_ &&
                flashlight_->GetBatteryRemaining()) {
                //当ててないときよりもゆっくり減少
                currentCharge_ -= decelerationRate_ * 0.6f * deltaTime * 60.0f;
            }
            else {
                currentCharge_ -= decelerationRate_ * deltaTime * 60.0f;
            }
        }

        currentCharge_ = std::clamp(currentCharge_, 0.0f, nitroThreshold_);

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

            //ノーマル判定
            if (currentCharge_ <= maxNormalCharge_) {
                OnNormalState();
                return;
            }

            //ニトロ判定
            if (currentCharge_ >= nitroThreshold_) {
                nitroAccumulateTimer_ += nitroAccelerationRate_;
                // 規定時間維持できたら発動
                if (nitroAccumulateTimer_ >= nitroChargeTime_) {
                    OnNitroState();
                }
            }
            else {
                nitroAccumulateTimer_ -= nitroDecelerationRate_;
                nitroAccumulateTimer_ = std::max(nitroAccumulateTimer_, 0.0f);
            }
        }
        break;
        case Trolley::State::Nitro:
        {

            currentSpeed_ = nitroSpeed_;

            stateTimer_++;
            // ニトロ終了判定
            if (stateTimer_ >= nitroDuration_) {
                RecoverFromNitro();
            }
        }
        break;
        case Trolley::State::Stop:
        {

        }
        break;
        default:
            break;
        }
    }
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

    float SoundConstant[kNitroBoostSECount] = {
        0.01f, 0.01f, 0.01f, 0.47f, 0.48f, 0.01f, 0.01f
    };

    float totalWeight = 0.0f;
    for (float w : SoundConstant) totalWeight += w;

    // 0 ～ totalWeight の間でランダム
    float r = rnd_.NextFloatRange(0.0f, totalWeight);

    int selected = 0;
    for (int i = 0; i < kNitroBoostSECount; ++i) {
        if (r < SoundConstant[i]) {
            selected = i;
            break;
        }
        r -= SoundConstant[i];
    }

    nitroBoostSESources_[selected].Play(false);
}

void Trolley::RecoverFromNitro()
{
    trollyState_ = State::Normal;
    currentCharge_ = batteryAfterNitro_;
    stateTimer_ = 0.0f;
    nitroAccumulateTimer_ = 0.0f;
    nitroFizzSESource_.Play(false);
}

void Trolley::RecoverFromBurst()
{
    trollyState_ = State::Normal;
    stateTimer_ = 0.0f;
    nitroAccumulateTimer_ = 0.0f;

    shakeRotation_ = Quaternion::identity;
    shakeOffset_ = Vector3::zero;
}

void Trolley::OnStopState()
{
    currentSpeed_ = 0.0f;
    currentCharge_ = 0.0f;
}

void Trolley::RecoverFromStop()
{
    currentSpeed_ = maxSpeed_;
    currentCharge_ = maxNormalCharge_;
}

void Trolley::UpdateSound() {
    if (normalSESource_.IsPlaying()) {
        // 速度がゼロなら音を止める
        if (currentSpeed_ <= 0.0f) {
            normalSESource_.Stop();
            nitroSESource_.Stop();
        }
        float t = std::clamp((currentSpeed_ - minSpeed_) / (maxSpeed_ - minSpeed_), 0.0f, 1.0f);
        t = t * t * t * t * t;
        float volume = std::clamp(currentSpeed_ / maxSpeed_, 0.0f, 1.0f);
        normalSESource_.SetVolume((1.0f - t) * volume);
        nitroSESource_.SetVolume(t * volume);

    }
    else {
        // 速度が出ているなら音を再生
        if (currentSpeed_ > 0.0f) {
            normalSESource_.Play(true);
            normalSESource_.SetVolume(0.0f);
            nitroSESource_.Play(true);
            nitroSESource_.SetVolume(0.0f);
        }
    }
}

void Trolley::OnBurstState()
{
    //trollyState_ = State::Burst;
    //stateTimer_ = 0.0f;
    //currentCharge_ = batteryAfterBurst_;
    //nitroAccumulateTimer_ = 0.0f;
    //burstSESource_.Play(false);
    //burstSESource_.SetVolume(1.5f);
    //burstSESource_.SetPitch(2.0f);
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
        //case State::Burst:
        //	stateStr = "BURST";
        //	stateColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // 赤
        //	break;
    }
    // STATEは見出しなので英語のまま強調
    ImGui::TextColored(stateColor, "STATE: %s", stateStr);

    // --- 速度表示 ---
    // 単位や意味がわかるように日本語を添える
    ImGui::SliderFloat("現在のトロッコ速度 (Speed)", &currentSpeed_, 0.0f, nitroSpeed_, "%.2f km/h");
    ImGui::SliderFloat("バッテリーを照らすライトの真ん中具合", &centerRate_, 0.0f, 1.0f, "%.2f");

    ImGui::Separator();

    // ゲージの色分け
    //if (trollyState_ == State::Burst)
    //	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // 赤
    //else
    if (currentCharge_ >= nitroThreshold_)
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.5f, 0.0f, 1.0f)); // オレンジ
    else
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // 緑

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
    ImGui::Checkbox("デバッグ：自然減速を停止", &isPause_);

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
			JSON_SAVE(nitroChargeTime_);
			JSON_SAVE(nitroDuration_);
			JSON_SAVE(batteryAfterNitro_);
			JSON_SAVE(batDecrease_);
			JSON_SAVE(nitroAccelerationRate_);
			JSON_SAVE(nitroDecelerationRate_);
			JSON_ROOT();

			JSON_OBJECT("TrolleyCollider");
			JSON_SAVE(trolleyColliderOffset_);
			JSON_SAVE(trolleyColliderHeight_);
			JSON_SAVE(trolleyColliderRadius_);
			JSON_SAVE(trolleyColliderQuaternion_);
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
			if (ImGui::TreeNode("トロッコ")) {
				static Vector3 rotate = { 0.0f,0.0f,0.0f };

				ImGui::DragFloat3("当たり判定オフセット", &trolleyColliderOffset_.x, 0.01f);
				ImGui::DragFloat("判定半径", &trolleyColliderRadius_, 0.01f);
				ImGui::DragFloat("高さ", &trolleyColliderHeight_, 0.01f);
				ImGui::DragFloat3("回転", &rotate.x, 0.01f);
				trolleyColliderQuaternion_ = Quaternion::MakeFromEulerAngle(rotate);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("バッテリー")) {
				for (uint8_t i = 0; i < BatteryNum; i++) {
					std::string key = "判定オフセット" + std::to_string(i) + ":";
					ImGui::DragFloat3(key.c_str(), &batteryOffsets_.at(i).x, 0.01f);
				}
				ImGui::DragFloat("判定半径 (Radius)", &batteryRadius_, 0.01f);
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("パラメータ設定 (Parameters)")) {

            ImGui::DragFloat("のろのろ進み始めるフレーム", &startFrame_, 0.01f);
            if (ImGui::TreeNode("トロッコの速度関連")) {
                ImGui::DragFloat("通常時の最高速度", &maxSpeed_, 0.01f);
                ImGui::DragFloat("通常時の最低速度", &minSpeed_, 0.01f);
                ImGui::DragFloat("ニトロ時の最高速度", &nitroSpeed_, 0.01f);
                ImGui::DragFloat("バースト時の最高速度", &burstSpeed_, 0.01f);



                ImGui::DragFloat("バット一体分の減少量", &batDecrease_, 0.1f, 0.0f);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("通常時のトロッコ関連")) {
                ImGui::DragFloat("バッテリーの許容量 (100%)", &maxNormalCharge_, 1.0f, 0.0f, nitroThreshold_);
                ImGui::Spacing();
                ImGui::DragFloat("バッテリーゲージの加算量", &accelerationRate_, 0.1f);
                ImGui::DragFloat("バッテリーゲージの減少量", &decelerationRate_, 0.1f);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("ニトロ関連のトロッコ関連")) {
                ImGui::DragFloat("ニトロ発動ライン", &nitroThreshold_, 1.0f, maxNormalCharge_);
                ImGui::Spacing();
                ImGui::DragFloat("ニトロ発動に必要な維持時間", &nitroChargeTime_, 0.1f, 0.0f, 600.0f);
                ImGui::DragFloat("ニトロ持続時間", &nitroDuration_, 0.1f, 0.0f, 10.0f);
                ImGui::Spacing();
                ImGui::DragFloat("ニトロゲージの加算量", &nitroAccelerationRate_, 0.1f);
                ImGui::DragFloat("ニトロゲージの減少量", &nitroDecelerationRate_, 0.1f);
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "イベント終了後の残量");
                ImGui::DragFloat("ニトロ終了後のバッテリー残量", &batteryAfterNitro_, 1.0f);
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

void Trolley::SetState(const State& state)
{

	switch (trollyState_)
	{
	case Trolley::State::Normal:
	{
	}
	break;
	case Trolley::State::Overcharge:
	{
	}
	break;
	case Trolley::State::Nitro:
	{
		RecoverFromNitro();
	}
	break;
	case Trolley::State::Stop:
	{
		RecoverFromStop();
	}
	break;
	default:
		break;
	}

    switch (state)
    {
    case Trolley::State::Normal:
    {
        OnNormalState();
    }
    break;
    case Trolley::State::Overcharge:
    {
        OnOverchargeState();
    }
    break;
    case Trolley::State::Nitro:
    {
        OnNitroState();
    }
    break;
    case Trolley::State::Stop:
    {
        OnStopState();
    }
    break;
    default:
        break;
    }
}

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
				case CollisionCategory::TROLLEYBATTERY:
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
				case CollisionCategory::TROLLEY:
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


void Trolley::SetIsActive(bool isActive) {
    model_.SetIsActive(isActive);
    trolleyUI_.SetIsActive(isActive);
}

void Trolley::Finalize() {
    if (normalSESource_.IsPlaying()) {
        normalSESource_.Stop();
    }
    if (nitroSESource_.IsPlaying()) {
        nitroSESource_.Stop();
    }
    if (burstSESource_.IsPlaying()) {
        burstSESource_.Stop();
    }
    if (crashSESource_.IsPlaying()) {
        crashSESource_.Stop();
    }
    for (uint32_t i = 0; i < kNitroBoostSECount; i++) {
        if (nitroBoostSESources_[i].IsPlaying()) {
            nitroBoostSESources_[i].Stop();
        }
    }
    if (nitroFizzSESource_.IsPlaying()) {
        nitroFizzSESource_.Stop();
    }
    if (chargeSESource_.IsPlaying()) {
        chargeSESource_.Stop();
    }
}