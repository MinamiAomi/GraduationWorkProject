#include "GameOverScene.h"

#include "PersistentData.h"
#include "GameScene.h"
#include "TitleScene.h"

#include "Framework/AssetManager.h"

namespace {
    const char* groundModelName = "GameOver_Movie_Ground";
    const char* trolleyModelName = "GameOver_Movie_Trolley";
}

void GameOverScene::OnInitialize() {
    persistentData_ = SceneManager::GetInstance()->GetPersistentData();
    input_ = Input::GetInstance();

    camera_ = std::make_shared<Camera>();
    camera_->SetPosition({ 0.0f, 0.0f, -5.0f });
    camera_->SetRotate(Quaternion::MakeFromEulerAngle({ 0.0f,0.0f,0.0f }));
    RenderManager::GetInstance()->SetCamera(camera_);

    RenderManager::GetInstance()->GetFogPostEffect().SetFogFactor(0.0f);

    directionalLights_.resize(kDirectionalLightCount);
    for (uint32_t i = 0; i < kDirectionalLightCount; ++i) {
        auto& directionalLight = directionalLights_[i];
        directionalLight = std::make_shared<DirectionalLight>();
        directionalLight->color = Color(1.0f, 1.0f, 1.0f);
        float t = Math::TwoPi * ((float)i / (float)kDirectionalLightCount);
        directionalLight->direction = { std::cos(t), -1.0f , std::sin(t) };
        directionalLight->intensity = 0.2f;
        directionalLight->isActive = true;
        //RenderManager::GetInstance()->GetLightManager().Add(directionalLight);
    }

    spotLight_ = std::make_shared<SpotLight>();
    spotLight_->direction = Vector3(0.0f, -1.0f, 0.0f);
    spotLight_->position = Vector3(0.0f, 5.0f, 20.0f);
    spotLight_->range = 50.0f;
    spotLight_->color = Color(1.0f, 1.0f, 1.0f);
    spotLight_->isActive = true;
    RenderManager::GetInstance()->GetLightManager().Add(spotLight_);

    collisionSystem_ = std::make_unique<CollisionSystem>();

    if (persistentData_) {
        persistentData_->score_->num = 0;
    }

    auto texture = AssetManager::GetInstance()->textureMap.Get("GameOver")->Get();

    sprite_.SetTexture(texture);
    sprite_.SetUVRect({ { 0.0f, 0.0f }, { 1.0f, 1.0f} }, Sprite::UVMode::UV);
    sprite_.SetPosition({ 1280.0f * 0.5f,720.0f * 0.5f });
    sprite_.SetScale({ 1280.0f , 720.0f });
    //sprite_.SetPre3DRender(true);

    selectTriangleLeft_ = std::make_unique<Diorama>();
    selectTriangleRight_ = std::make_unique<Diorama>();

    Vector3 leftPos = Vector3(-5.8f, -1.3f, 5.0f);
    Vector3 leftFor = -Vector3::Cross(camera_->GetPosition() - leftPos, Vector3::up);
    selectTriangleLeft_->Initialize("Triangle", leftPos, Quaternion::MakeLookRotation(leftFor));
    selectTriangleLeft_->SetRoateAxis(Diorama::ZAxis);
    Vector3 rightPos = Vector3(5.8f, -1.3f, 5.0f);
    Vector3 rightFor = Vector3::Cross(camera_->GetPosition() - rightPos, Vector3::up);
    selectTriangleRight_->Initialize("Triangle", rightPos, Quaternion::MakeLookRotation(rightFor));
    selectTriangleRight_->SetRoateAxis(Diorama::ZAxis);
    selectTriangleRight_->SetRotateRate(-1.0f);

    collisionSystem_->RegisterCollider(selectTriangleLeft_->GetCollider());
    collisionSystem_->RegisterCollider(selectTriangleRight_->GetCollider());

    flashlight_ = std::make_unique<Flashlight>();
    //flashlight_->SetRailAnimationPlayer(railAnimationPlayer_.get());
    flashlight_->Initialize(&camera_->GetTransform(), camera_.get(), false);
    collisionSystem_->RegisterCollider(flashlight_->GetCollider());

    parentTransform_.translate = { 0.0f, -5.0f, 20.0f };
    parentTransform_.rotate = Quaternion::MakeFromEulerAngle({ 0.0f, 0.0f * Math::ToRadian, 0.0f });
    parentTransform_.UpdateMatrix();

    trolleyTransform_.translate = { 0.0f, -1.8f, .0f };;
    trolleyTransform_.SetParent(&parentTransform_, false);
    trolleyTransform_.UpdateMatrix();

    auto assetManager = AssetManager::GetInstance();
    trolleyModelInstance_.SetModel(assetManager->modelMap.Get(trolleyModelName)->Get());
    trolleyModelInstance_.SetWorldMatrix(trolleyTransform_.worldMatrix);

    groundModelInstnace_.SetModel(assetManager->modelMap.Get(groundModelName)->Get());
    groundModelInstnace_.SetWorldMatrix(parentTransform_.worldMatrix);

    seAudioSource_ = AssetManager::GetInstance()->soundMap.Get("SE_GAMEOVER")->Get();
    seAudioSource_.Play(false);
    seAudioSource_.SetVolume(0.5f);
}

void GameOverScene::OnUpdate() {

    flashlight_->Update();
    camera_->UpdateMatrices();

    selectTriangleLeft_->Update();
    selectTriangleRight_->Update();

    parentTransform_.rotate *= Quaternion::MakeFromEulerAngle({ 0.0f, 0.25f * Math::ToRadian, 0.0f });
    parentTransform_.UpdateMatrix();
    trolleyTransform_.UpdateMatrix();
    groundModelInstnace_.SetWorldMatrix(trolleyTransform_.worldMatrix);
    trolleyModelInstance_.SetWorldMatrix(parentTransform_.worldMatrix);

    if (selectTriangleLeft_->GetIsActive()) {
        SceneManager::GetInstance()->ChangeScene<GameScene>(false);
        
    }
    else if (selectTriangleRight_->GetIsActive()) {
        SceneManager::GetInstance()->ChangeScene<TitleScene>(false);
    }

    collisionSystem_->CheckCollisions();
}

void GameOverScene::OnFinalize() {

}

