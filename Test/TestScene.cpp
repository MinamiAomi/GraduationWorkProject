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

    sofaTransform_.rotate = Engine::GetIMUDevice()->GetOrientation();
    sofaTransform_.UpdateMatrix();
    sofaModel_.SetWorldMatrix(sofaTransform_.worldMatrix);
}

void TestScene::OnFinalize() {

}
