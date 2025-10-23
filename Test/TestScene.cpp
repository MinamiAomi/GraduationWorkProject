#include "TestScene.h"

#include "Framework/Engine.h"
#include "Input/Input.h"
#include "Graphics/RenderManager.h"
#include "Framework/AssetManager.h"
#include "Graphics/Core/TextureLoader.h"
#include "LevelLoader.h"
#include "Input/IMUDevice.h"

void TestScene::OnInitialize() {

    sunLight_ = std::make_shared<DirectionalLight>();
    sunLight_->direction = -Vector3::unitY;
    RenderManager::GetInstance()->SetSunLight(sunLight_);


   LevelLoader::Load("Resources/scene.json", *Engine::GetGameObjectManager());

    sofaModel_.SetModel(AssetManager::GetInstance()->modelMap.Get("sofa")->Get());
    sofaTransform_.translate = { 0.0f, 5.0f, 0.0f };
    sofaTransform_.rotate = Quaternion::identity;
    sofaTransform_.scale = Vector3::one;
}

void TestScene::OnUpdate() {

    Engine::GetGameObjectManager()->Update();

    if (Input::GetInstance()->IsKeyTrigger(DIK_V)) {
        sofaTransform_.rotate = Quaternion::identity;;
    }

    // 1. 必要なデータを取得
    Vector3 gyro = Engine::GetIMUDevice()->GetGyroscope(); // ジャイロ (rad/s と仮定)
    Vector3 g = { -gyro.y, -gyro.z, gyro.x };

    Quaternion current_q = sofaTransform_.rotate;        // 現在のソファの回転
    //float dt = 1.0f / 60.0f;                 // 経過時間 (秒)

    // 2. クォータニオンの微分方程式 (正しい式)
    //    ジャイロ(g)によって、現在の回転(current_q)がどう変化するか(qDot)を計算
    Quaternion qDot;
    qDot.w = 0.5f * (-current_q.x * g.x - current_q.y * g.y - current_q.z * g.z);
    qDot.x = 0.5f * (current_q.w * g.x + current_q.y * g.z - current_q.z * g.y);
    qDot.y = 0.5f * (current_q.w * g.y - current_q.x * g.z + current_q.z * g.x);
    qDot.z = 0.5f * (current_q.w * g.z + current_q.x * g.y - current_q.y * g.x);

    // 3. 積分 (変化率を現在の回転に加算)
    //    新しい回転 = 現在の回転 + (変化率 * 時間)
    Quaternion new_q;
    new_q.w = current_q.w + qDot.w;
    new_q.x = current_q.x + qDot.x;
    new_q.y = current_q.y + qDot.y;
    new_q.z = current_q.z + qDot.z;

    // 4. 正規化 (必須)
    //    計算誤差でクォータニオンの長さが1でなくなるのを防ぐ
    // 5. 最終的な回転を適用
    sofaTransform_.rotate = new_q.Normalized();
    sofaTransform_.UpdateMatrix();
    sofaModel_.SetWorldMatrix(sofaTransform_.worldMatrix);
}

void TestScene::OnFinalize() {

}
