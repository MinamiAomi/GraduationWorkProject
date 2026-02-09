#include "GameClearScene.h"


#include "PersistentData.h"
#include "StageSelectScene.h"
#include "TitleScene.h"

#include "Framework/AssetManager.h"

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
	sprite_.SetPre3DRender(true);

	selectTriangleLeft_ = std::make_unique<Diorama>();
	selectTriangleRight_ = std::make_unique<Diorama>();

	selectTriangleLeft_->Initialize("Triangle", Vector3(-6.1f, -1.3f, 5.0f));
	selectTriangleRight_->Initialize("Triangle", Vector3(6.1f, -1.3f, 5.0f));

	collisionSystem_->RegisterCollider(selectTriangleLeft_->GetCollider());
	collisionSystem_->RegisterCollider(selectTriangleRight_->GetCollider());

	flashlight_ = std::make_unique<Flashlight>();
	//flashlight_->SetRailAnimationPlayer(railAnimationPlayer_.get());
	flashlight_->Initialize(&camera_->GetTransform(), camera_.get());
	collisionSystem_->RegisterCollider(flashlight_->GetCollider());

    parentTransform_.translate = { 5.0f, -10.0f, 40.0f };
	parentTransform_.rotate = Quaternion::MakeFromEulerAngle({0.0f, -135.0f * Math::ToRadian, 0.0f});
	parentTransform_.UpdateMatrix();

	auto assetManager = AssetManager::GetInstance();
	trolley_ = std::make_unique<AnimationModel>();
    trolley_->transform.SetParent(&parentTransform_, false);
	trolley_->modelInstance.SetModel(assetManager->modelMap.Get("GameClearTrolley")->Get());
	trolley_->animation = assetManager->animationMap.Get("GameClearTrolleyAnim");

	goal_ = std::make_unique<AnimationModel>();
    goal_->transform.SetParent(&parentTransform_, false);
	goal_->modelInstance.SetModel(assetManager->modelMap.Get("GameClearGoalAndRail")->Get());
}

void GameClearScene::OnUpdate() {


	flashlight_->Update();
	camera_->UpdateMatrices();


	selectTriangleLeft_->Update();
	selectTriangleRight_->Update();

	const AnimationSet& anime = trolley_->animation->Get()->GetAnimation("Animation");
	auto it = anime.nodeAnimations.find("Trolley");

	trolley_->animationTime += 0.016f / anime.duration;
	if (trolley_->animationTime >= 1.0f) {
		trolley_->animationTime = 0.0f;
	}

	if (it != anime.nodeAnimations.end()) {
		const NodeAnimation& nodeAnim = it->second;

		trolley_->transform.translate = CalculateValue(nodeAnim.translate, trolley_->animationTime);
		trolley_->transform.rotate = CalculateValue(nodeAnim.rotate, trolley_->animationTime);
		trolley_->transform.scale = CalculateValue(nodeAnim.scale, trolley_->animationTime);
	}

	trolley_->transform.UpdateMatrix();
	trolley_->modelInstance.SetWorldMatrix(trolley_->transform.worldMatrix);


    goal_->transform.UpdateMatrix();
    goal_->modelInstance.SetWorldMatrix(goal_->transform.worldMatrix);

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
