#include "GameClearScene.h"

#include "PersistentData.h"
#include "StageSelectScene.h"

void GameClearScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	if (persistentData_) {
		persistentData_->score_->num;
	}
	input_ = Input::GetInstance();

	std::shared_ptr<Texture> texture = Texture::Load("Resources/Title.png");

	sprite_.SetTexture(texture);
	sprite_.SetTexcoordRect({ 0.0f, 0.0f }, { 1280.0f, 720.0f });
	sprite_.SetPosition({ 1280.0f / 2.0f, 720.0f / 2.0f });
	sprite_.SetScale({ 1280.0f, 720.0f });

}

void GameClearScene::OnUpdate() {

	if (input_->IsKeyTrigger(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene<StageSelectScene>();
	}
}

void GameClearScene::OnFinalize() {

}
