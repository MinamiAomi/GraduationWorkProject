#include "GameOverScene.h"

#include "PersistentData.h"
#include "StageSelectScene.h"
#include "TitleScene.h"

#include "Framework/AssetManager.h"

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
        RenderManager::GetInstance()->GetLightManager().Add(directionalLight);
    }

    collisionSystem_ = std::make_unique<CollisionSystem>();

    if (persistentData_) {
        persistentData_->score_->num = 0;
    }

    auto texture = AssetManager::GetInstance()->textureMap.Get("GameOver")->Get();

    sprite_.SetTexture(texture);
    sprite_.SetUVRect({ { 0.0f, 0.0f }, { 1.0f, 1.0f} }, Sprite::UVMode::UV);
    sprite_.SetPosition({ 1280.0f * 0.5f,720.0f * 0.5f });
    sprite_.SetScale({ 1280.0f , 720.0f });
    sprite_.SetPre3DRender(true);

    selectTriangleLeft_ = std::make_unique<Diorama>();
    selectTriangleRight_ = std::make_unique<Diorama>();

    selectTriangleLeft_->Initialize("Triangle", Vector3(-6.1f, -1.3f, 5.0f));
    selectTriangleLeft_->SetRoateAxis(Diorama::YAxis);
    selectTriangleRight_->Initialize("Triangle", Vector3(6.1f, -1.3f, 5.0f));
    selectTriangleRight_->SetRoateAxis(Diorama::YAxis);

    collisionSystem_->RegisterCollider(selectTriangleLeft_->GetCollider());
    collisionSystem_->RegisterCollider(selectTriangleRight_->GetCollider());

    flashlight_ = std::make_unique<Flashlight>();
    //flashlight_->SetRailAnimationPlayer(railAnimationPlayer_.get());
    flashlight_->Initialize(&camera_->GetTransform(), camera_.get());
    collisionSystem_->RegisterCollider(flashlight_->GetCollider());

    parentTransform_.translate = { -10.0f, -5.0f, 50.0f };
    parentTransform_.rotate = Quaternion::MakeFromEulerAngle({ 0.0f, 75.0f * Math::ToRadian, 0.0f });
    parentTransform_.UpdateMatrix();

    auto assetManager = AssetManager::GetInstance();
    trolley_ = std::make_unique<AnimationModel>();
    trolley_->transform.SetParent(&parentTransform_, false);
    trolley_->modelInstance.SetModel(assetManager->modelMap.Get("GameOverTrolley")->Get());
    trolley_->animation = assetManager->animationMap.Get("GameOverTrolleyAnim");

    oodama_ = std::make_unique<AnimationModel>();
    oodama_->transform.SetParent(&parentTransform_, false);
    oodama_->modelInstance.SetModel(assetManager->modelMap.Get("GameOverOODAMA")->Get());
    oodama_->animation = assetManager->animationMap.Get("GameOverOODAMAAnim");

    rail_ = std::make_unique<AnimationModel>();
    rail_->transform.SetParent(&parentTransform_, false);
    rail_->modelInstance.SetModel(assetManager->modelMap.Get("GameOverRail")->Get());

    seAudioSource_ = AssetManager::GetInstance()->soundMap.Get("SE_GAMEOVER")->Get();
    seAudioSource_.Play(false);
    seAudioSource_.SetVolume(0.5f);
}

void GameOverScene::OnUpdate() {

    flashlight_->Update();
    camera_->UpdateMatrices();

    selectTriangleLeft_->Update();
    selectTriangleRight_->Update();

    const AnimationSet& anime = trolley_->animation->Get()->GetAnimation("Animation");
    auto it = anime.nodeAnimations.find("Trolley");

    if (trolley_->animationTime <= 1.0f) {
        trolley_->animationTime += 0.016f / anime.duration;
    }
    else {
        trolley_->animationTime = 1.0f;
    }

    if (it != anime.nodeAnimations.end()) {
        const NodeAnimation& nodeAnim = it->second;

        trolley_->transform.translate = CalculateValue(nodeAnim.translate, trolley_->animationTime);
        trolley_->transform.rotate = CalculateValue(nodeAnim.rotate, trolley_->animationTime);
        trolley_->transform.scale = CalculateValue(nodeAnim.scale, trolley_->animationTime);
    }

    trolley_->transform.UpdateMatrix();
    trolley_->modelInstance.SetWorldMatrix(trolley_->transform.worldMatrix);


    const AnimationSet& caveAnime = oodama_->animation->Get()->GetAnimation("Animation");
    it = caveAnime.nodeAnimations.find("OODAMA");

    if (oodama_->animationTime <= 1.0f) {
        oodama_->animationTime += 0.016f / anime.duration;
    }
    else {
        oodama_->animationTime = 1.0f;
    }


    if (it != caveAnime.nodeAnimations.end()) {
        const NodeAnimation& nodeAnim = it->second;

        oodama_->transform.translate = CalculateValue(nodeAnim.translate, oodama_->animationTime);
        oodama_->transform.rotate = CalculateValue(nodeAnim.rotate, oodama_->animationTime);
        oodama_->transform.scale = CalculateValue(nodeAnim.scale, oodama_->animationTime);
    }

    oodama_->transform.UpdateMatrix();
    oodama_->modelInstance.SetWorldMatrix(oodama_->transform.worldMatrix);

    rail_->transform.UpdateMatrix();
    rail_->modelInstance.SetWorldMatrix(rail_->transform.worldMatrix);

    if (selectTriangleLeft_->GetIsActive()) {
        SceneManager::GetInstance()->ChangeScene<StageSelectScene>(false);
    }
    else if (selectTriangleRight_->GetIsActive()) {
        SceneManager::GetInstance()->ChangeScene<TitleScene>(false);
    }

    collisionSystem_->CheckCollisions();
}

void GameOverScene::OnFinalize() {

}

