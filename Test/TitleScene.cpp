#include "TitleScene.h"

#include "PersistentData.h"
#include "StageSelectScene.h"
#include "Graphics/RenderManager.h"
#include "Engine/Framework/AssetManager.h"

void TitleScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	if (persistentData_) {
		
	}
	input_ = Input::GetInstance();
	camera_ = std::make_shared<Camera>();
	camera_->UpdateMatrices();

	RenderManager::GetInstance()->SetCamera(camera_);

	stoneModels_[0] = std::make_unique<ModelInstance>();
	stoneTransforms_[0] = std::make_unique<Transform>();
	
	auto assetManager = AssetManager::GetInstance();
	stoneModels_[0]->SetModel(assetManager->modelMap.Get("O")->Get());
	stoneTransforms_[0]->translate = Vector3{0.0f,0.0f,0.0f};
	stoneTransforms_[0]->UpdateMatrix();
	stoneModels_[0]->SetWorldMatrix(stoneTransforms_[0]->worldMatrix);
}

void TitleScene::OnUpdate() {
	if (input_->IsKeyTrigger(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene<StageSelectScene>();
	}
}

void TitleScene::OnFinalize() {

}
