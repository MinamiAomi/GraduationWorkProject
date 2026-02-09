#include "TitleScene.h"

#include "PersistentData.h"
#include "StageSelectScene.h"
#include "Graphics/RenderManager.h"
#include "Engine/Framework/AssetManager.h"
#include "Engine/File/JsonConverter.h"

void TitleScene::OnInitialize() {
	auto assetManager = AssetManager::GetInstance();
	const auto& model = assetManager->modelMap.Get("TitleTrolley")->Get();
	const auto& caveModel = assetManager->modelMap.Get("TitleStage")->Get();

	trolley_ = std::make_unique<AnimationModel>();
	trolley_->modelInstance.SetModel(model);
	trolley_->animation = assetManager->animationMap.Get("trolleyAnim");
	trolley_->skeleton = std::make_unique<Skeleton>();
	trolley_->skeleton->Create(model);
	trolley_->modelInstance.SetSkeleton(trolley_->skeleton);

	cave_ = std::make_unique<AnimationModel>();
	cave_->modelInstance.SetModel(caveModel);
	/*cave_->animation = assetManager->animationMap.Get("TitleStageAnim");
	cave_->skeleton = std::make_unique<Skeleton>();
	cave_->skeleton->Create(caveModel);
	cave_->modelInstance.SetSkeleton(cave_->skeleton);*/

	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	if (persistentData_) {
		
	}
	input_ = Input::GetInstance();

    camera_ = std::make_shared<DebugCamera>();
	camera_->Initialize();
    RenderManager::GetInstance()->SetCamera(camera_->GetCamera());

    deviceOptionsUI_ = std::make_unique<DeviceOptionsUI>();
    deviceOptionsUI_->Initialize();
}

void TitleScene::OnUpdate() {
	RenderManager::GetInstance()->SetCamera(camera_->GetCamera());

	camera_->Update();
    deviceOptionsUI_->Update();

	trolley_->skeleton->ApplyAnimation(trolley_->animation->Get()->GetAnimation("\u5186\u67f1.002Action"), trolley_->animationTime);
	trolley_->skeleton->Update();
	trolley_->transform.scale = { 10.0f,10.f,10.0f };
	trolley_->transform.UpdateMatrix();
	trolley_->modelInstance.SetWorldMatrix(trolley_->transform.worldMatrix);


	trolley_->animationTime += 0.04f;
	if (trolley_->animationTime >= 1.0f) {
		trolley_->animationTime = 0.0f;
	}

	//cave_->skeleton->ApplyAnimation(cave_->animation->Get()->GetAnimation("CaveAction"), cave_->animationTime);
	//cave_->skeleton->Update();
	//cave_->transform.scale = { 10.0f,10.f,10.0f };
	cave_->transform.UpdateMatrix();
	cave_->modelInstance.SetWorldMatrix(cave_->transform.worldMatrix);

	cave_->animationTime += 0.04f;
	if (cave_->animationTime >= 1.0f) {
		cave_->animationTime = 0.0f;
	}
}

void TitleScene::OnFinalize() {

}
