#include "TitleScene.h"

#include "PersistentData.h"
#include "StageSelectScene.h"
#include "Graphics/RenderManager.h"

void TitleScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	if (persistentData_) {
		
	}
	input_ = Input::GetInstance();

	std::shared_ptr<Texture> texture = Texture::Load("Resources/Title.png");

	sprite_.SetTexture(texture);
	sprite_.SetTexcoordRect({ 0.0f, 0.0f }, { 1280.0f, 720.0f });
	sprite_.SetPosition({ 1280.0f / 2.0f, 720.0f / 2.0f });
	sprite_.SetScale({ 1280.0f, 720.0f });

}

void TitleScene::OnUpdate() {
	if (input_->IsKeyTrigger(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene<StageSelectScene>();
	}
}

void TitleScene::OnFinalize() {

}
