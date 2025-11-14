#include "GameOverScene.h"

#include "PersistentData.h"
#include "StageSelectScene.h"

void GameOverScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	if (persistentData_) {
		persistentData_->score_->num;
	}
	input_ = Input::GetInstance();

	std::shared_ptr<Texture> texture = Texture::Load("Resources/GameOver.png");

	sprite_.SetTexture(texture);
	sprite_.SetTexcoordRect({ 0.0f, 0.0f }, { 1280.0f, 720.0f });
	sprite_.SetPosition({ 1280.0f / 2.0f, 720.0f / 2.0f });
	sprite_.SetScale({ 1280.0f, 720.0f });
}

void GameOverScene::OnUpdate() {

	if (input_->IsKeyTrigger(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene<StageSelectScene>();
	}
}

void GameOverScene::OnFinalize() {

}
