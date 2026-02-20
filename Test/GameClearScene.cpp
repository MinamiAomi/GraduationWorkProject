#include "GameClearScene.h"


#include "PersistentData.h"
#include "StageSelectScene.h"
#include "TitleScene.h"

#include "Framework/AssetManager.h"

namespace {
	const char* oodamaModelName = "GameClear_Movie_OODAMA";
	const char* groundModelName = "GameClear_Movie_Stage";
	const char* trolleyModelName = "GameClear_Movie_Trolley";

	const char* oodamaAnimName = "GameClear_Movie_OODAMA_Anim";
	const char* trolleyAnimName = "GameClear_Movie_Trolley_Anim";
}

void GameClearScene::OnInitialize() {
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

	auto texture = AssetManager::GetInstance()->textureMap.Get("GameClear")->Get();

	sprite_.SetTexture(texture);
	sprite_.SetUVRect({ { 0.0f, 0.0f }, { 1.0f, 1.0f} }, Sprite::UVMode::UV);
	sprite_.SetPosition({ 1280.0f * 0.5f,720.0f * 0.5f });
	sprite_.SetScale({ 1280.0f , 720.0f });
	//sprite_.SetPre3DRender(true);

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

    parentTransform_.translate = { 3.0f, -15.0f, 70.0f };
	parentTransform_.rotate = Quaternion::MakeFromEulerAngle({0.0f, 15.0f * Math::ToRadian, 0.0f});
	parentTransform_.UpdateMatrix();

	auto assetManager = AssetManager::GetInstance();
    auto trolleyAnimAsset = assetManager->animationMap.Get(trolleyAnimName);
    auto trolleyAnimationName = "Animation";
    auto nodeName = "Trolley";
    trolleyAnimationTransform_.transform.SetParent(&parentTransform_, false);
	trolleyAnimationTransform_.Initialize(trolleyAnimAsset, trolleyAnimationName, nodeName);
    trolleyModelInstance_.SetModel(assetManager->modelMap.Get(trolleyModelName)->Get());

    auto oodamaAnimAsset = assetManager->animationMap.Get(oodamaAnimName);
    auto oodamaAnimationName = "Animation";
    auto oodamaNodeName = "OODAMA";
    oodamaAnimationTransform_.transform.SetParent(&parentTransform_, false);
    oodamaAnimationTransform_.Initialize(oodamaAnimAsset, oodamaAnimationName, oodamaNodeName);
    oodamaModelInstance_.SetModel(assetManager->modelMap.Get(oodamaModelName)->Get());

    stageModelInstance_.SetModel(assetManager->modelMap.Get(groundModelName)->Get());

    seAudioSource_ = AssetManager::GetInstance()->soundMap.Get("SE_GAMECLEAR")->Get();
    seAudioSource_.Play(false);
    seAudioSource_.SetVolume(0.5f);
}

void GameClearScene::OnUpdate() {


	flashlight_->Update();
	camera_->UpdateMatrices();


	selectTriangleLeft_->Update();
	selectTriangleRight_->Update();

	parentTransform_.UpdateMatrix();

	float deltaTime = 1.0f / 60.0f;
    trolleyAnimationTransform_.Update(deltaTime);
    oodamaAnimationTransform_.Update(deltaTime);

    trolleyModelInstance_.SetWorldMatrix(trolleyAnimationTransform_.transform.worldMatrix);
    oodamaModelInstance_.SetWorldMatrix(oodamaAnimationTransform_.transform.worldMatrix);
    stageModelInstance_.SetWorldMatrix(parentTransform_.worldMatrix);

	if (selectTriangleLeft_->GetIsActive()) {
		SceneManager::GetInstance()->ChangeScene<StageSelectScene>(false);
	}
	else if (selectTriangleRight_->GetIsActive()) {
		SceneManager::GetInstance()->ChangeScene<TitleScene>(false);
	}

	collisionSystem_->CheckCollisions();
}

void GameClearScene::OnFinalize() {

}
