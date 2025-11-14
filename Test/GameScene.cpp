#include "GameScene.h"

#include "PersistentData.h"
#include "Framework/Engine.h"
#include "Input/Input.h"
#include "Graphics/RenderManager.h"
#include "Framework/AssetManager.h"
#include "Graphics/Core/TextureLoader.h"
#include "LevelLoader.h"

#include "RailCameraLoader.h"
#include "RailCameraConverter.h"
#include "RailCameraDebugUtils.h"

#include "GameClearScene.h"
#include "GameOverScene.h"


void GameScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	input_ = Input::GetInstance();

	LevelLoader::Load("Resources/scene.json", *Engine::GetGameObjectManager());

	std::shared_ptr<Texture> texture = Texture::Load("Resources/Sprite-0001.dds");

	sprite_.SetTexture(texture);
	sprite_.SetTexcoordRect({ 0.0f, 0.0f }, { 256.0f, 256.0f });
	sprite_.SetPosition({ 200.0f, 200.0f });
	sprite_.SetScale({ 200.0f, 200.0f });

	railCameraModel_.SetModel(AssetManager::GetInstance()->modelMap.Get("sphere")->Get());


	auto animationData = RailCameraSystem::AnimationLoader::LoadAnimation("Resources/RailCamera/railCamera.json");
	if (animationData) {
		railCameraController_ = std::make_unique<RailCameraSystem::RailCameraController>
			(
				std::make_shared<const RailCameraSystem::RailCameraAnimation>(*animationData)
			);
		railCameraController_->Play();
	}

	camera_ = std::make_shared<Camera>();
}

void GameScene::OnUpdate() {

	Engine::GetGameObjectManager()->Update();

	railCameraController_->Update(0.01667f);
	auto transform = railCameraController_->GetCurrentTransform();
	transform = RailCameraSystem::RailCameraConverter::ConvertToLeftHand(transform);
	transform.UpdateMatrix();
	railCameraModel_.SetWorldMatrix(transform.worldMatrix);



	camera_->SetPosition(transform.translate);
	camera_->SetRotate(transform.rotate);
	camera_->UpdateMatrices();
	railCameraController_->GetCurrentFrame();

#ifdef _DEBUG
	auto vertices = RailCameraSystem::RailCameraDebugUtils::CalculateFrustum(camera_->GetViewMatrix(), camera_->GetProjectionMatrix());

	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();

	Vector4 color = { 0.0f,1.0f,1.0f,1.0 };

	//近平面
	lineDrawer.AddLine(vertices[0], vertices[1], color); // 左下 -> 右下
	lineDrawer.AddLine(vertices[1], vertices[2], color); // 右下 -> 右上
	lineDrawer.AddLine(vertices[2], vertices[3], color); // 右上 -> 左上
	lineDrawer.AddLine(vertices[3], vertices[0], color); // 左上 -> 左下


	//遠平面
	lineDrawer.AddLine(vertices[4], vertices[5], color);
	lineDrawer.AddLine(vertices[5], vertices[6], color);
	lineDrawer.AddLine(vertices[6], vertices[7], color);
	lineDrawer.AddLine(vertices[7], vertices[4], color);

	//近平面と遠平面をつなぐ線
	lineDrawer.AddLine(vertices[0], vertices[4], color);
	lineDrawer.AddLine(vertices[1], vertices[5], color);
	lineDrawer.AddLine(vertices[2], vertices[6], color);
	lineDrawer.AddLine(vertices[3], vertices[7], color);

	if (input_->IsKeyTrigger(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene<GameClearScene>();
	}

#endif // _DEBUG

	//
	//RenderManager::GetInstance()->SetCamera(camera_);
}

void GameScene::OnFinalize() {

}
