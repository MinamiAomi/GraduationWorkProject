#include "StageSelectScene.h"

#include "PersistentData.h"
#include "GameScene.h"


void StageSelectScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	input_ = Input::GetInstance();

	if (persistentData_) {
		persistentData_->score_->num = 0;
	}

	std::shared_ptr<Texture> texture = Texture::Load("Resources/StageSelect.png");

	sprite_.SetTexture(texture);
	sprite_.SetTexcoordRect({ 0.0f, 0.0f }, { 1280.0f, 720.0f });
	sprite_.SetPosition({ 1280.0f / 2.0f, 720.0f / 2.0f });
	sprite_.SetScale({ 1280.0f, 720.0f });

}

void StageSelectScene::OnUpdate() {
	if (input_->IsKeyTrigger(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene<GameScene>();
	}

}

void StageSelectScene::OnFinalize() {

}
