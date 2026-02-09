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

	cave_ = std::make_unique<AnimationModel>();
	cave_->modelInstance.SetModel(caveModel);
	cave_->animation = assetManager->animationMap.Get("TitleStageAnim");

	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	if (persistentData_) {
		
	}
	input_ = Input::GetInstance();

    camera_ = std::make_shared<Camera>();
	camera_->SetRotate(Quaternion{ 0.00682486175f, 0.210466623f, -0.00146935426f,0.977576077f });
	camera_->SetPosition({ -90.0401764f ,4.65209627f, -21.6976643f });
    RenderManager::GetInstance()->SetCamera(camera_);

  //  deviceOptionsUI_ = std::make_unique<DeviceOptionsUI>();
  //  deviceOptionsUI_->Initialize();

	std::shared_ptr<Texture> texture = Texture::Load("Resources/titleLog.png");

	title_.SetTexture(texture);
	title_.SetUVRect({ { 0.0f, 0.0f }, { 1280.0f, 720.0f } });
	title_.SetPosition({ 1280.0f / 2.0f, 720.0f / 2.0f });
	title_.SetScale({ 1280.0f, 720.0f });

	collisionSystem_ = std::make_unique<CollisionSystem>();


	flashlight_ = std::make_unique<Flashlight>();
	flashlight_->Initialize(&camera_->GetTransform(), camera_.get());
	collisionSystem_->RegisterCollider(flashlight_->GetCollider());


}

void TitleScene::OnUpdate() {
	camera_->UpdateMatrices();
	flashlight_->Update();
	collisionSystem_->CheckCollisions();
	RenderManager::GetInstance()->SetCamera(camera_);

  //  deviceOptionsUI_->Update();

	const AnimationSet& anime = trolley_->animation->Get()->GetAnimation("\u5186\u67f1.002Action");
	auto it = anime.nodeAnimations.find("TitleTrollory");

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


	const AnimationSet& caveAnime = cave_->animation->Get()->GetAnimation("CaveAction");
	it = caveAnime.nodeAnimations.find("Cave");

	cave_->animationTime += 0.016f / caveAnime.duration;
	if (cave_->animationTime >= 1.0f) {
		cave_->animationTime = 0.0f;
	}

	if (it != caveAnime.nodeAnimations.end()) {
		const NodeAnimation& nodeAnim = it->second;

		cave_->transform.translate = CalculateValue(nodeAnim.translate, cave_->animationTime);
		cave_->transform.rotate = CalculateValue(nodeAnim.rotate, cave_->animationTime);
		cave_->transform.scale = CalculateValue(nodeAnim.scale, cave_->animationTime);
	}

	cave_->transform.UpdateMatrix();
	cave_->modelInstance.SetWorldMatrix(cave_->transform.worldMatrix);

	Input* input = Input::GetInstance();
	if (input->IsKeyTrigger(DIK_SPACE)) {
		// ゲームスタート
		SceneManager::GetInstance()->ChangeScene<StageSelectScene>(true);
	}
}

void TitleScene::OnFinalize() {

}
