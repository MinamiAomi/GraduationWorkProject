#include "GameOverScene.h"

#include "PersistentData.h"
#include "StageSelectScene.h"
#include "TitleScene.h"

#include "Framework/AssetManager.h"

void GameOverScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	input_ = Input::GetInstance();

	camera_ = std::make_shared<DebugCamera>();
	camera_->Initialize();
	RenderManager::GetInstance()->SetCamera(camera_->GetCamera());

	RenderManager::GetInstance()->GetFogPostEffect().SetFogFactor(0.0f);

	collisionSystem_ = std::make_unique<CollisionSystem>();

	if (persistentData_) {
		persistentData_->score_->num = 0;
	}

	auto texture = AssetManager::GetInstance()->textureMap.Get("GameOver")->Get();

	sprite_.SetTexture(texture);
	sprite_.SetUVRect({ { 0.0f, 0.0f }, { 1.0f, 1.0f} }, Sprite::UVMode::UV);
	sprite_.SetPosition({ 1280.0f * 0.5f,720.0f * 0.5f });
	sprite_.SetScale({ texture->GetSize() });

	selectTriangleLeft_ = std::make_unique<Diorama>();
	selectTriangleRight_ = std::make_unique<Diorama>();

	selectTriangleLeft_->Initialize("Triangle", Vector3(-2.3f, -1.0f, 1.0f));
	selectTriangleRight_->Initialize("Triangle", Vector3(2.3f, -1.0f, 1.0f));

	collisionSystem_->RegisterCollider(selectTriangleLeft_->GetCollider());
	collisionSystem_->RegisterCollider(selectTriangleRight_->GetCollider());

	flashlight_ = std::make_unique<Flashlight>();
	//flashlight_->SetRailAnimationPlayer(railAnimationPlayer_.get());
	flashlight_->Initialize(&camera_->GetCamera()->GetTransform(), camera_->GetCamera().get());
	collisionSystem_->RegisterCollider(flashlight_->GetCollider());
}

void GameOverScene::OnUpdate() {

	camera_->Update();

	flashlight_->Update();

	selectTriangleLeft_->Update();
	selectTriangleRight_->Update();

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

