#include "Flashlight.h"

#include "Engine/Graphics/RenderManager.h"
#include "Engine/Graphics/GameWindow.h"
#include "Engine/Input/Input.h"
#include "Engine/File/JsonConverter.h"

#include <algorithm>
#include "Engine/Framework/Engine.h"
#include "Engine/Framework/AssetManager.h"
#include "RailAnimationPlayer.h"
#include "GameSystem.h"
#include "LightDeviceInput.h"

Flashlight::Flashlight()
{
    auto assetManager = AssetManager::GetInstance();
    lightModel_.SetModel(assetManager->modelMap.Get("flashlight")->Get());

    spotLight_ = std::make_shared<SpotLight>();
    spotLight_->position = transform_.worldMatrix.GetTranslate();
    spotLight_->direction = transform_.worldMatrix.GetForward();
    spotLight_->color = Color::white;
    spotLight_->intensity = 2.5f;
    spotLight_->range = lightRange_ + 100.0f;
    spotLight_->angle = fovAngle_ * 0.5f;
    spotLight_->falloffStartAngle = fovAngle_ * 0.45f;
    spotLight_->decay = 1.0f;
    RenderManager::GetInstance()->GetLightManager().Add(spotLight_);

    collider_ = std::make_shared<ConeCollider>(
        CollisionCategory::FLASHLIGHT,
        (CollisionCategory::LIGHT | CollisionCategory::ENEMY | CollisionCategory::ITEM | CollisionCategory::PLAYER),
        Vector3::zero,
        0.0f, 0.0f,

        Quaternion::identity);

    flashlightUI_.SetFlashlight(this);
}

void Flashlight::Initialize(const Transform* parentTransform, const Camera* parentCamera)
{
    const Vector3 kFlashLightOffset = { 0.0f,0.0f,0.0f };

    parentTransform_ = parentTransform;
    parentCamera_ = parentCamera;
    transform_.SetParent(parentTransform, false);

    transform_.translate = kFlashLightOffset;
    transform_.UpdateMatrix();

    lightModel_.SetWorldMatrix(transform_.worldMatrix);
    lightModel_.SetIsActive(false);

    JSON_OPEN("Resources/Data/Flashlight/flashlight.json");
    JSON_OBJECT("light");
    JSON_LOAD(fovAngle_);
    JSON_LOAD(lightRange_);
    JSON_ROOT();
    JSON_OBJECT("battery");
    JSON_LOAD(maxBattery_);
    JSON_LOAD(addBattery_);
    JSON_LOAD(subBattery_);
    JSON_LOAD(startFrame_);
    JSON_LOAD(isLighting_);
    JSON_ROOT();
    JSON_CLOSE();

    sphericalAngleX_ = 0.0f;
    sphericalAngleY_ = -5.0f * Math::ToRadian;

    battery_ = maxBattery_;

    isHitFlashlight_ = false;

    flashlightUI_.Initialize();
}

void Flashlight::Update()
{
#ifdef _DEBUG
#endif // _DEBUG
    //DebugMove();


    Move();

    // ライトモデルの更新
    //transform_.rotate = Quaternion::MakeFromEulerAngle(Vector3(sphericalAngleY_, sphericalAngleX_, 0.0f));
    transform_.UpdateMatrix();
    lightModel_.SetWorldMatrix(transform_.worldMatrix);

    UpdateLightPower();

    UpdateCollision();

    spotLight_->position = transform_.worldMatrix.GetTranslate();
    spotLight_->direction = transform_.worldMatrix.GetForward();
    spotLight_->range = lightRange_ + 100.0f;
    spotLight_->angle = fovAngle_ * 0.5f;
    spotLight_->isActive = isLighting_;

    flashlightUI_.Update();
#ifdef _DEBUG
    DrawImGui();
#endif // _DEBUG
}

void Flashlight::UpdateCollision()
{

    isHitFlashlight_ = false;
    Quaternion colliderRotation = transform_.worldMatrix.GetRotate() * Quaternion::MakeForXAxis(-90.0f * Math::ToRadian);

    Vector3 heightOffset = colliderRotation * Vector3(0.0f, -lightRange_, 0.0f);

    collider_->quaternion = colliderRotation;
    collider_->height = lightRange_;
    collider_->center = transform_.worldMatrix.GetTranslate() + heightOffset;

    collider_->radius = std::tan(fovAngle_ * 0.5f) * lightRange_;


    if (!collider_->GetCollidedWith().empty()) {
        for (const auto& collider : collider_->GetCollidedWith()) {

            switch (collider->categoryBits)
            {
            case CollisionCategory::NONE:
                break;
            case CollisionCategory::PLAYER:
                break;
            case CollisionCategory::FLASHLIGHT:
                break;
            case CollisionCategory::LIGHT:
            {
                battery_ += addBattery_;
                battery_ = std::clamp(battery_, 0.0f, maxBattery_);
                isLighting_ = true;
                isHitFlashlight_ = true;
            }
            break;
            case CollisionCategory::ENEMY:
                break;
            case CollisionCategory::ITEM:
                break;
            case CollisionCategory::ALL:
                break;
            default:
                break;
            }
        }
    }
}

void Flashlight::UpdateLightPower()
{
#ifdef _DEBUG
    if (!isDebug_) {
#endif // _DEBUG
        if (railAnimationPlayer_->GetCurrentFrame() >= startFrame_ &&
            !isHitFlashlight_) {
            battery_ -= subBattery_;
            battery_ = std::clamp(battery_, 0.0f, maxBattery_);
        }

        //20.0%未満からちかちか
        if (battery_ < maxBattery_ * 0.2f) {

            blinkTimer_ += (1.0f / 60.0f);

            static const int pattern[] = {
                        1, 1, 1, 0, 1, 1, 1, 1, 0, 1,
                        1, 0, 0, 1, 1, 0, 1, 0, 0, 0,
                        0, 0, 0, 1, 0, 0, 0, 0, 1, 1,
                        0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
                        0, 0, 1, 1, 0, 0, 1, 1, 1, 1,
            };
            int patternLength = sizeof(pattern) / sizeof(pattern[0]);

            float stepTime = 0.1f;

            int index = (int)(blinkTimer_ / stepTime) % patternLength;

            isLighting_ = (pattern[index] == 1);
        }
        else {
            isLighting_ = true;
        }
#ifdef _DEBUG
    }
#endif // _DEBUG
}

void Flashlight::SpotLightDebugDraw() const
{
    const uint32_t segments = 16;
    std::vector<Vector3> vertices(segments);
    Vector3 apex = Vector3::zero * transform_.worldMatrix;

    float halfFovAngle = fovAngle_ * 0.5f;
    float radius = lightRange_ * std::tan(halfFovAngle);

    for (uint32_t i = 0; i < segments; ++i) {
        float angle = (static_cast<float>(i) / segments) * Math::TwoPi;
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);
        float z = lightRange_;
        vertices[i] = Vector3(x, y, z) * transform_.worldMatrix;
    }

    auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
    Vector4 color = { 1.0f, 1.0f, 0.0f, 1.0 };

    for (uint32_t i = 0; i < segments; ++i) {
        uint32_t  j = (i + 1) % segments;

        lineDrawer.AddLine(apex, vertices[i], color); // 側面
        lineDrawer.AddLine(vertices[i], vertices[j], color); // 底面
    }
}

void Flashlight::Move() {
    GameSystem* gameSystem = GameSystem::GetInstance();
    Input* input = Input::GetInstance();

    auto playDevice = gameSystem->GetPlayDevice();
    switch (playDevice)
    {
    case GameSystem::PlayDevice::KeyboardMouse: {

        float mousePositionX = std::clamp(static_cast<float>(input->GetMousePosition().x), 0.0f, static_cast<float>(Engine::kWindowWidth));
        float mousePositionY = std::clamp(static_cast<float>(input->GetMousePosition().y), 0.0f, static_cast<float>(Engine::kWindowHeight));

        float ndcX = (2.0f * mousePositionX / static_cast<float>(Engine::kWindowWidth)) - 1.0f;
        float ndcY = 1.0f - (2.0f * mousePositionY) / static_cast<float>(Engine::kWindowHeight);

        Vector3 clipCoords = { ndcX, ndcY, 1.0f };
        Vector3 target = parentCamera_->GetProjectionMatrix().Inverse().ApplyTransformWDivide(clipCoords);

        transform_.rotate = Quaternion::MakeLookRotation(target);

        break;
    }
    case GameSystem::PlayDevice::LightDevice: {
        LightDeviceInput* lightDeviceInput = LightDeviceInput::GetInstance();
        if (lightDeviceInput->GetConnectionState() == LightDeviceInput::ConnectionState::Connected) {
            if (input->IsKeyTrigger(DIK_R)) {
                lightDeviceInput->ResetOrientation();
            }
            Quaternion deviceOrientation = lightDeviceInput->GetOrientation();
            transform_.rotate = deviceOrientation;
        }
        break;
    }
    }


}

void Flashlight::DebugMove() {
    const float anglerSpeed = 0.5f * Math::ToRadian;
    Input* input = Input::GetInstance();
    if (input->IsKeyPressed(DIK_W)) {
        sphericalAngleY_ -= anglerSpeed;
    }
    if (input->IsKeyPressed(DIK_S)) {
        sphericalAngleY_ += anglerSpeed;
    }
    if (input->IsKeyPressed(DIK_D)) {
        sphericalAngleX_ += anglerSpeed;
    }
    if (input->IsKeyPressed(DIK_A)) {
        sphericalAngleX_ -= anglerSpeed;
    }
    transform_.rotate = Quaternion::MakeFromEulerAngle(Vector3(sphericalAngleY_, sphericalAngleX_, 0.0f));
}

#ifdef _DEBUG
void Flashlight::DrawImGui()
{
    static bool isDebugDraw = true;
    // =================================================================================
        // 1. ステータス表示 & 操作ウィンドウ (リアルタイム用)
        //    プレイ中に残量をいじってテストできるようにSliderに変更
        // =================================================================================
    ImGui::Begin("Flashlight Status");

    // --- 状態表示 ---
    if (isLighting_) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.5f, 1.0f), "STATE: LIGHT ON");
    }
    else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "STATE: OFF");
    }

    ImGui::Separator();

    // --- バッテリー操作スライダー ---
    // 残量割合計算
    float batteryRatio = 0.0f;
    if (maxBattery_ > 0.0f) {
        batteryRatio = battery_ / maxBattery_;
    }

    // 残量に応じて「スライダーのつまみ」の色を変える (視認性確保)
    ImVec4 sliderColor;
    if (batteryRatio < 0.2f)
        sliderColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // 赤 (ピンチ)
    else if (batteryRatio < 0.5f)
        sliderColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // 黄 (注意)
    else
        sliderColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f); // 緑 (安全)

    // スライダーの色を変更して描画
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, sliderColor);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, sliderColor);

    // ここをProgressBarからSliderFloatに変更し、直接 battery_ をいじれるようにしました
    ImGui::Text("現在の残量 (操作可)");
    ImGui::SliderFloat("##BatterySlider", &battery_, 0.0f, maxBattery_, "%.1f");

    ImGui::PopStyleColor(2); // 色設定を戻す

    // 数値をテキストでも補足
    ImGui::SameLine();
    ImGui::TextColored(sliderColor, "%.0f%%", batteryRatio * 100.0f);


    // --- デバッグスイッチ ---
    ImGui::Separator();
    ImGui::Checkbox("デバッグ：システム有効化 (IsDebug)", &isDebug_);
    ImGui::Checkbox("デバッグ：当たり判定描画 (Show Collider)", &isDebugDraw);

    ImGui::End();

    // =================================================================================
    // 2. パラメータ調整ウィンドウ (GameSceneタブ内)
    //    各種数値の設定。ツリー構造で見やすく整理。
    // =================================================================================
    ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);

    if (ImGui::TreeNode("懐中電灯制御 (FlashLight)")) {
        // 保存ボタン (配置場所を考慮)
        if (ImGui::Button("Save", ImVec2(-1.0f, 0.0f))) {
            JSON_OPEN("Resources/Data/Flashlight/flashlight.json");
            JSON_OBJECT("light");
            JSON_SAVE(distanceFromCamera_);
            JSON_SAVE(fovAngle_);
            JSON_SAVE(lightRange_);
            JSON_ROOT();
            JSON_OBJECT("battery");
            JSON_SAVE(maxBattery_);
            JSON_SAVE(addBattery_);
            JSON_SAVE(subBattery_);
            JSON_SAVE(startFrame_);
            JSON_SAVE(isLighting_);
            JSON_ROOT();
            JSON_CLOSE();
        }

        ImGui::Separator();
        // --------------------------------------------------------
        // ライトの基本設定
        // --------------------------------------------------------
        if (ImGui::TreeNode("ライト挙動設定 (Light Settings)")) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "位置・角度調整");
            ImGui::DragFloat3("基本座標オフセット", &transform_.translate.x, 0.1f);
            ImGui::DragFloat("カメラからの距離", &distanceFromCamera_, 0.1f, 0.0f, 50.0f, "%.1fm");

            // 角度計算 (Radian <-> Degree)
            Vector2 sphericalAnglesDegrees = { Math::ToDegree * sphericalAngleX_, Math::ToDegree * sphericalAngleY_ };
            if (ImGui::SliderFloat2("照射角度 (X, Y)", &sphericalAnglesDegrees.x, -90.0f, 90.0f, "%.1f deg")) {
                sphericalAngleX_ = Math::ToRadian * sphericalAnglesDegrees.x;
                sphericalAngleY_ = Math::ToRadian * sphericalAnglesDegrees.y;
            }

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "ライト性能");

            // FOV計算
            float fovAngleDegree = fovAngle_ * Math::ToDegree;
            if (ImGui::DragFloat("照射範囲 (FOV)", &fovAngleDegree, 1.0f, 1.0f, 90.0f, "%.1f deg")) {
                fovAngle_ = fovAngleDegree * Math::ToRadian;
            }

            ImGui::DragFloat("光の届く距離 (Range)", &lightRange_, 0.1f, 1.0f, 100.0f, "%.1fm");

            ImGui::TreePop();
        }

        // --------------------------------------------------------
        // バッテリー設定
        // --------------------------------------------------------
        if (ImGui::TreeNode("バッテリー設定 (Battery Params)")) {

            ImGui::DragFloat("最大容量 (Max)", &maxBattery_, 1.0f, 1.0f, 1000.0f);

            // 単位や用途がわかるように補足
            ImGui::DragFloat("回復量 (Add)", &addBattery_, 1.0f, 0.0f, maxBattery_, "+%.3f / item");
            ImGui::DragFloat("消費速度 (Drain)", &subBattery_, 0.01f, 0.0f, 10.0f, "-%.3f / frame");
            ImGui::DragFloat("消費が始まるフレーム (Start)", &startFrame_, 0.1f, float(railAnimationPlayer_->GetRailAnimationDate()->railMetaData_.startFrame), float(railAnimationPlayer_->GetRailAnimationDate()->railMetaData_.endFrame), "%.2f / frame目");


            ImGui::TreePop();
        }

        // --------------------------------------------------------
        // 内部コンポーネント
        // --------------------------------------------------------
        // 必要に応じてツリー化
        if (ImGui::TreeNode("内部スポットライト (Inner SpotLight)")) {
            spotLight_->DrawImGui("SpotLight");
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
    ImGui::End();

    // スポットライトの当たり判定描画 (デバッグフラグがONの時のみ)
    if (isDebugDraw) {
        SpotLightDebugDraw();
    }
}
#endif // _DEBUG