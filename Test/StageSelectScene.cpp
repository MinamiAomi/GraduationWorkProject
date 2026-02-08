#include "StageSelectScene.h"

#include "PersistentData.h"
#include "GameScene.h"
#include "LevelManager.h"

#include "Framework/AssetManager.h"

void StageSelectScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	input_ = Input::GetInstance();

	camera_ = std::make_shared<DebugCamera>();
	camera_->Initialize();
	RenderManager::GetInstance()->SetCamera(camera_->GetCamera());

	if (persistentData_) {
		persistentData_->score_->num = 0;
	}

	auto texture = AssetManager::GetInstance()->textureMap.Get("StageSelect")->Get();

	sprite_.SetTexture(texture);
	sprite_.SetUVRect({ { 0.0f, 0.0f }, { 1.0f, 1.0f} }, Sprite::UVMode::UV);
	sprite_.SetPosition({ 1280.0f * 0.5f,720.0f * 0.5f });
	sprite_.SetScale({ texture->GetSize() });

	level1_.Initialize("DioramaLevel1", Vector3(-2.0f, -0.5f, 0.0f));
	level2_.Initialize("DioramaLevel2", Vector3(2.0f, -0.5f, 0.0f));
}

void StageSelectScene::OnUpdate() {

	camera_->Update();

	level1_.Update();
	level2_.Update();

	/*if (input_->IsKeyTrigger(DIK_1)) {
		LevelManager::GetInstance()->SetLevel(LevelManager::Level::LEVEL1);
		SceneManager::GetInstance()->ChangeScene<GameScene>();
	}
	else if (input_->IsKeyTrigger(DIK_2)) {
		LevelManager::GetInstance()->SetLevel(LevelManager::Level::LEVEL2);
		SceneManager::GetInstance()->ChangeScene<GameScene>();
	}*/
}

void StageSelectScene::OnFinalize() {

}
