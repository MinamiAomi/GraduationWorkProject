#include "Flashlight.h"

#include "Engine/Graphics/RenderManager.h"
#include "Engine/Graphics/GameWindow.h"
#include "Engine/Input/Input.h"

#include "Engine/Framework/AssetManager.h"

void Flashlight::Initialize(const Transform* parentTransform, const Camera* parentCamera)
{

    parentTransform_ = parentTransform;
    parentCamera_ = parentCamera;
    flashLightOffset_ = { 0.0f,0.0f,1.0f };
    transform_.SetParent(parentTransform);
    transform_.translate = flashLightOffset_;
    transform_.UpdateMatrix();

    lightTransform_.SetParent(&transform_);
    lightTransform_.UpdateMatrix();
    auto assetManager = AssetManager::GetInstance();
    lightModel_.SetModel(assetManager->modelMap.Get("flashlight")->Get());
    lightModel_.SetWorldMatrix(lightTransform_.worldMatrix);

    moveSpeed_ = 0.01f;

    Vector3 initialLightPos = transform_.worldMatrix.GetTranslate();
    Vector3 initialLightDir = parentTransform_ ? parentTransform_->worldMatrix.GetForward() : Vector3::unitZ;
    focusT_ = 0.5f;

    focusDistance_ = std::lerp(focusT_, MinFocusDistance, MaxFocusDistance);
    focusPoint_ = { 0.0f, 10.0f, focusDistance_ };

    focusPoint_ = initialLightPos + initialLightDir.Normalized() * focusDistance_;
    range_ = std::lerp(focusT_, MinFocusRange, MaxFocusRange);
    SetProperties(20.0f, range_);

}

void Flashlight::Update()
{
#ifdef _DEBUG
    ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);
    if (ImGui::TreeNode("FlashLight")) {
        ImGui::DragFloat("DistanceFromCamera", &distanceFromCamera_, 0.1f, 0.0f, 50.0f);
        Vector2 sphericalAnglesDegrees = { Math::ToDegree * sphericalAngleX_, Math::ToDegree * sphericalAngleY_ };
        ImGui::SliderFloat2("SphericalAngles", &sphericalAnglesDegrees.x, -90, 90);
        sphericalAngleX_ = Math::ToRadian * sphericalAnglesDegrees.x;
        sphericalAngleY_ = Math::ToRadian * sphericalAnglesDegrees.y;
        float fovAngleDegree = fovAngle_ * Math::ToDegree;
        ImGui::DragFloat("FovAngle", &fovAngleDegree, 1.0f, 1.0f, 90.0f);
        fovAngle_ = fovAngleDegree * Math::ToRadian;
        ImGui::SliderFloat("LightRange", &lightRange_, 1.0f, 100.0f);
        ImGui::TreePop();
    }
    ImGui::End();
#endif // _DEBUG



    UpdateLightShape();
    Move();
    UpdateFocusPoint();

    transform_.translate = flashLightOffset_;
    transform_.rotate = Quaternion::identity;
    transform_.UpdateMatrix();

    // ライトモデルの更新
    lightTransform_.rotate = Quaternion::MakeFromEulerAngle({ sphericalAngleY_, sphericalAngleX_, 0.0f });
    lightTransform_.translate = lightTransform_.rotate * (Vector3::unitZ * distanceFromCamera_);
    lightTransform_.UpdateMatrix();
    lightModel_.SetWorldMatrix(lightTransform_.worldMatrix);

#ifdef _DEBUG
    // スポットライトの当たり判定描画
    SpotLightDebugDraw();
#endif // _DEBUG



//#ifdef _DEBUG
//    Vector3 lightWorldPos = transform_.worldMatrix.GetTranslate();
//    // RenderManagerやLineDrawerの取得方法はエンジン依存
//    auto renderManager = RenderManager::GetInstance();
//    if (renderManager) {
//        auto& lineDrawer = renderManager->GetLineDrawer();
//        Vector4 color = { 1.0f, 1.0f, 0.0f, 1.0 }; // 黄色
//        // ★ ライトの位置から、マウスで動かした focusPoint_ へ線を引く
//        lineDrawer.AddLine(lightWorldPos, focusPoint_, color);
//
//        // 円錐描画 (ライトの位置からfocusPoint_への方向に基づいて描画)
//        Vector3 lightDirection = focusPoint_ - lightWorldPos;
//        if (lightDirection.LengthSquare() > 0.0001f) { // ゼロベクトルでなければ
//            lightDirection = lightDirection.Normalized();
//
//            // 円錐の向きを表す回転を計算 (Upベクトルはカメラの上を使うと安定)
//            Vector3 upVector = parentTransform_->worldMatrix.GetUp(); // カメラの上方向
//            Quaternion coneRotation = Quaternion::MakeLookRotation(lightDirection, upVector);
//
//            // 回転から基底ベクトルを取得 (Matrix経由またはQuaternionから直接)
//            Matrix4x4 coneMatrix = Matrix4x4::MakeRotation(coneRotation); // 回転行列を作成 (仮)
//            Vector3 rightVec = coneMatrix.GetRight().Normalized();
//            Vector3 upVec = coneMatrix.GetUp().Normalized(); // 円錐自身のローカルUp
//
//            // 円錐の底面の半径と中心を計算
//            float coneRadius = range_ * std::tan(fovAngleRadians_);
//            // ★ 円錐の底面中心は focusPoint_ ではなく、ライトの位置からrange_進んだ点
//            Vector3 coneBaseCenter = lightWorldPos + lightDirection * range_;
//            int segments = 16; // 円周の分割数
//            for (int i = 0; i < segments; ++i) {
//                float angle1 = static_cast<float>(i) / segments * Math::TwoPi;
//                float angle2 = static_cast<float>(i + 1) / segments * Math::TwoPi;
//                // 底面の円周上の点を計算
//                Vector3 p1_local = (rightVec * std::cos(angle1) + upVec * std::sin(angle1)) * coneRadius;
//                Vector3 p2_local = (rightVec * std::cos(angle2) + upVec * std::sin(angle2)) * coneRadius;
//                Vector3 p1 = coneBaseCenter + p1_local;
//                Vector3 p2 = coneBaseCenter + p2_local;
//                // 側面 (光源から円周へ)
//                lineDrawer.AddLine(lightWorldPos, p1, { 0.5f, 0.5f, 0.0f, 1.0f }); // 暗い黄色
//                // 底面の円周
//                lineDrawer.AddLine(p1, p2, color); // 明るい黄色
//            }
//        }
//    }
//#endif // _DEBUG

}

Vector3 Flashlight::GetDirection() const
{
    Vector3 lightWorldPos = transform_.worldMatrix.GetTranslate();
    Vector3 direction = focusPoint_ - lightWorldPos;
    if (direction.LengthSquare() > 0.0001f) {
        return direction.Normalized();
    }
    else {
        return parentTransform_ ? parentTransform_->worldMatrix.GetForward().Normalized() : Vector3::unitZ;
    }
}

Vector3 Flashlight::GetPosition() const
{
    return transform_.worldMatrix.GetTranslate();
}

void Flashlight::Move()
{
    const auto& input = Input::GetInstance();
    Vector3 moveInput = Vector3::zero;
    if (input->IsKeyPressed(DIK_A)) {
        moveInput.x = -1.0f;
    }
    if (input->IsKeyPressed(DIK_D)) {
        moveInput.x = 1.0f;
    }
    if (input->IsKeyPressed(DIK_W)) {
        moveInput.y = 1.0f;
    }
    if (input->IsKeyPressed(DIK_S)) {
        moveInput.y = -1.0f;
    }
    // 入力がある場合のみ処理
    if (moveInput.LengthSquare() > 0.0f) {
        moveInput = moveInput.Normalized();

        Vector3 localMove = moveInput * moveSpeed_;
        flashLightOffset_ += localMove;


        const Matrix4x4& parentWorldMatrix = parentTransform_->worldMatrix;
        Vector3 right = parentWorldMatrix.GetRight();
        Vector3 up = parentWorldMatrix.GetUp();
        Vector3 forward = parentWorldMatrix.GetForward();
        Vector3 worldMoveVector =
            right * moveInput.x +
            up * moveInput.y +
            forward * moveInput.z;

        // 現在のオフセットに移動量を加算
        focusPoint_ += worldMoveVector * moveSpeed_;
    }
}

void Flashlight::UpdateFocusPoint()
{
    const auto& input = Input::GetInstance();
    Vector2 mousePos = { float(input->GetMousePosition().x),float(input->GetMousePosition().y) };

    // スクリーン座標からワールド方向へ
    const auto& gameWindowInstance = GameWindow::GetInstance();
    const Vector2& screenArea = { float(gameWindowInstance->GetClientWidth()),float(gameWindowInstance->GetClientHeight()) };


    Vector3 worldDirection = UnprojectScreenToWorld(mousePos, screenArea);

    // ライトの現在位置を取得
    Vector3 lightWorldPos = parentTransform_->worldMatrix * flashLightOffset_;

    //カメラが死なないように
    if (worldDirection.LengthSquare() > std::numeric_limits<float>::epsilon()) {
        focusPoint_ = lightWorldPos + worldDirection.Normalized() * focusDistance_;
    }
    else {
        focusPoint_ = lightWorldPos + (parentTransform_ ? parentTransform_->worldMatrix.GetForward().Normalized() : Vector3::unitZ) * range_;
    }
}

void Flashlight::UpdateLightShape()
{
    const auto& input = Input::GetInstance();
    if (std::abs(float(input->GetMouseWheel())) > 0) {
        focusT_ += float(input->GetMouseWheel()) * 0.001f;
        focusT_ = std::clamp(focusT_, 0.0f, 1.0f);

    }
    range_ = std::lerp(MaxFocusRange, MinFocusRange, focusT_);
    focusDistance_ = std::lerp(MinFocusDistance, MaxFocusDistance, focusT_);
}

void Flashlight::SpotLightDebugDraw() const
{
    const uint32_t segments = 16;
    std::vector<Vector3> vertices(segments);
    Vector3 apex = Vector3::zero * lightTransform_.worldMatrix;

    float halfFovAngle = fovAngle_ * 0.5f;
    float radius = lightRange_ * std::tan(halfFovAngle);

    for (uint32_t i = 0; i < segments; ++i) {
        float angle = (static_cast<float>(i) / segments) * Math::TwoPi;
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);
        float z = lightRange_;
        vertices[i] = Vector3(x, y, z) * lightTransform_.worldMatrix;
    }

    auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
    Vector4 color = { 1.0f, 1.0f, 0.0f, 1.0 };

    for (uint32_t i = 0; i < segments; ++i) {
        uint32_t  j = (i + 1) % segments;

        lineDrawer.AddLine(apex, vertices[i], color); // 側面
        lineDrawer.AddLine(vertices[i], vertices[j], color); // 底面
    }
}

Vector3 Flashlight::UnprojectScreenToWorld(const Vector2& mousePos, const Vector2& screenArea) const
{
    if (screenArea.x < 0 || screenArea.y < 0) {
        return parentTransform_->worldMatrix.GetForward();
    }
    Vector2 ndc = { (2.0f * mousePos.x / screenArea.x - 1.0f),(1.0f - (2.0f * mousePos.y / screenArea.y)) };

    Matrix4x4 inverseViewProjection = parentCamera_->GetViewProjectionMatrix().Inverse();
    Matrix4x4 viewportMatrix = Matrix4x4::MakeViewport(0.0, 0.0f, screenArea.x, screenArea.y, parentCamera_->GetNearClip(), parentCamera_->GetFarClip());

    Vector4 ndcNear = { ndc.x,ndc.y,0.0f,1.0f };
    Vector4 ndcFar = { ndc.x,ndc.y,1.0f,1.0f };

    Vector4 worldNear = ndcNear * inverseViewProjection;
    Vector4 worldFar = ndcFar * inverseViewProjection;

    worldNear.x /= worldNear.w;
    worldNear.y /= worldNear.w;
    worldNear.z /= worldNear.w;

    worldFar.x /= worldFar.w;
    worldFar.y /= worldFar.w;
    worldFar.z /= worldFar.w;

    Vector3 worldDirection = Vector3(worldFar.x - worldNear.x, worldFar.y - worldNear.y, worldFar.z - worldNear.z);

    return worldDirection.Normalized();
}

void Flashlight::SetProperties(float fovAngleDegrees, float range)
{
    fovAngleRadians_ = fovAngleDegrees * 0.5f * Math::ToRadian;
    range_ = std::max(0.1f, range);
}

