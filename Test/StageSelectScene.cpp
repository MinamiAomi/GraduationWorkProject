#include "StageSelectScene.h"

#include "PersistentData.h"
#include "GameScene.h"
#include "LevelManager.h"

void StageSelectScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	input_ = Input::GetInstance();

	camera_ = std::make_shared<DebugCamera>();
	camera_->Initialize();
	RenderManager::GetInstance()->SetCamera(camera_->GetCamera());

	if (persistentData_) {
		persistentData_->score_->num = 0;
	}

	std::shared_ptr<Texture> texture = Texture::Load("Resources/StageSelect.png");

	/*sprite_.SetTexture(texture);
	sprite_.SetUVRect({ { 0.0f, 0.0f }, { 1280.0f, 720.0f } });
	sprite_.SetPosition({ 1280.0f / 2.0f, 720.0f / 2.0f });
	sprite_.SetScale({ 1280.0f, 720.0f });*/

	level1_.Initialize("DioramaLevel1",Vector3(-20.0f,-10.0f,50.0f));
	level2_.Initialize("DioramaLevel2", Vector3(20.0f, -10.0f, 50.0f));
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
