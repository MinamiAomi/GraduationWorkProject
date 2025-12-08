#include "GameScene.h"

#include "PersistentData.h"
#include "Framework/Engine.h"
#include "Input/Input.h"
#include "Graphics/RenderManager.h"
#include "Framework/AssetManager.h"
#include "Graphics/Core/TextureLoader.h"

#include "RailLoader.h"
#include "RailConverter.h"
#include "RailDebugUtils.h"

#include "GameClearScene.h"
#include "GameOverScene.h"
#include "SceneObjectLoader.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

void GameScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	input_ = Input::GetInstance();

	camera_ = std::make_shared<Camera>();
#pragma region CollisionSystem
	collisionSystem_ = std::make_unique<CollisionSystem>();
#pragma endregion

#pragma region RailSystem
	auto animationData = RailSystem::AnimationLoader::LoadAnimation("Resources/RailCamera/railCamera.json");
	if (animationData) {
		railAnimationPlayer_ = std::make_unique<RailSystem::RailAnimationPlayer>
			(
				std::make_shared<const RailSystem::RailAnimation>(*animationData)
			);
		//カメラ再生
		railAnimationPlayer_->Play();
	}

#pragma endregion

#pragma region Flashlight
	flashlight_ = std::make_unique<Flashlight>();
	flashlight_->Initialize(&camera_->GetTransform(), camera_.get());
	collisionSystem_->RegisterCollider(flashlight_->GetCollider());
#pragma endregion

#pragma region Trolley
	trolley_ = std::make_unique<Trolley>();
	trolley_->SetParent(railAnimationPlayer_->GetTransform());
	trolley_->SetRailAnimationPlayer(railAnimationPlayer_.get());
	trolley_->SetFlashlight(flashlight_.get());
	trolley_->Initialize();
	collisionSystem_->RegisterCollider(trolley_->GetCollider());
#pragma endregion

#pragma region RailCameraSystem
	railCameraSystem_ = std::make_unique<RailSystem::RailCameraSystem>();
	railCameraSystem_->SetRailAnimationPlayer(railAnimationPlayer_.get());
	railCameraSystem_->SetParent(trolley_->GetTransform());
	railCameraSystem_->Initialize();
#pragma endregion

#pragma region SceneObjectSystem
	sceneObjectManager_ = std::make_unique<SceneObjectSystem::SceneObjectManager>();

	sceneObjectManager_->Initialize();

	auto result = SceneObjectSystem::SceneLoader::LoadSceneFromFile("Resources/StaticMesh/Mint_staticMesh.json");

	sceneObjectManager_->CreateObjects(result);

	//Colliderセット
	for (const auto& collider : sceneObjectManager_->GetPointLightObjects()) {
		collisionSystem_->RegisterCollider(collider->collider);
	}
	for (const auto& collider : sceneObjectManager_->GetEmitterObjects()) {
		collisionSystem_->RegisterCollider(collider->collider);
	}
	for (const auto& collider : sceneObjectManager_->GetEnemyObjects()) {
		collisionSystem_->RegisterCollider(collider->collider);
	}

#pragma endregion

#ifdef _DEBUG
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
#endif // _DEBUG

}

void GameScene::OnUpdate() {
	float deltaTime = 1.0f / 60.0f;

#pragma region RailSystem
	//一周終わったかどうか
	if (railAnimationPlayer_->IsFinished()) {
		//SceneObjectsリセット
		sceneObjectManager_->ResetObjects();

		//Colliderセット
		for (const auto& collider : sceneObjectManager_->GetPointLightObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
		for (const auto& collider : sceneObjectManager_->GetEmitterObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
		for (const auto& collider : sceneObjectManager_->GetEnemyObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
		railAnimationPlayer_->Loop();
	}
	//現在のスピードを代入
	railAnimationPlayer_->SetPlaybackSpeed(trolley_->GetTrollySpeed());
	//更新
	railAnimationPlayer_->Update(deltaTime);
#pragma endregion
#pragma region Trolley
	trolley_->Update(deltaTime);
#pragma endregion

#pragma region Flashlight
	flashlight_->Update();
#pragma endregion

#pragma region RailCameraSystem
	railCameraSystem_->Update(deltaTime);
	camera_->SetFov(railCameraSystem_->GetFov());
	camera_->SetRotate(railCameraSystem_->GetWorldRotation());
	camera_->SetPosition(railCameraSystem_->GetWorldTranslate());
#pragma endregion
	camera_->UpdateMatrices();
	RenderManager::GetInstance()->SetCamera(camera_);


#pragma region SceneObjectSystem
	sceneObjectManager_->Update();
#pragma endregion
#pragma region CollisionSystem
	collisionSystem_->CheckCollisions();
#pragma endregion
#ifdef _DEBUG
	static bool isDebugCamera = false;
	ImGui::Begin("GameScene");
	//デバックカメラ
	if (ImGui::Checkbox("DebugCamera", &isDebugCamera)) {
		debugCamera_->SetTransform(railCameraSystem_->GetTransform());
	}
	if (isDebugCamera) {
		debugCamera_->Update();

		//線描画
		auto vertices = RailSystem::RailDebugUtils::CalculateFrustum(camera_->GetViewMatrix(), camera_->GetProjectionMatrix());

		auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();

		Vector4 color = { 0.0f,1.0f,1.0f,1.0 };

		//近平面
		lineDrawer.AddLine(vertices[0], vertices[1], color);
		lineDrawer.AddLine(vertices[1], vertices[2], color);
		lineDrawer.AddLine(vertices[2], vertices[3], color);
		lineDrawer.AddLine(vertices[3], vertices[0], color);


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

		RenderManager::GetInstance()->SetCamera(debugCamera_->GetCamera());
	}
	ImGui::End();

	if (input_->IsKeyTrigger(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene<GameClearScene>();
	}
#endif 
}

void GameScene::OnFinalize() {

}
