#include "GameScene.h"

#include "PersistentData.h"
#include "Framework/Engine.h"
#include "Input/Input.h"
#include "Graphics/RenderManager.h"
#include "Framework/AssetManager.h"
#include "Graphics/Core/TextureLoader.h"

#include "RailCameraLoader.h"
#include "RailCameraConverter.h"
#include "RailCameraDebugUtils.h"

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

#pragma region Flashlight
	flashlight_ = std::make_unique<Flashlight>();
	flashlight_->Initialize(&camera_->GetTransform(), camera_.get());
#pragma endregion

#pragma region RailCameraSystem
	auto animationData = RailCameraSystem::AnimationLoader::LoadAnimation("Resources/RailCamera/railCamera.json");
	if (animationData) {
		railCameraController_ = std::make_unique<RailCameraSystem::RailCameraController>
			(
				std::make_shared<const RailCameraSystem::RailCameraAnimation>(*animationData)
			);
		//カメラ再生
		railCameraController_->Play();
	}
#pragma endregion

#pragma region SceneObjectSystem
	sceneObjectManager_ = std::make_unique<SceneObjectSystem::SceneObjectManager>();

	sceneObjectManager_->Initialize();

	auto result = SceneObjectSystem::SceneLoader::LoadSceneFromFile("Resources/StaticMesh/Mint_staticMesh.json");

	sceneObjectManager_->CreateObjects(result);
#pragma endregion
	collisionSystem_ = std::make_unique<CollisionSystem>();
	test1_ = std::make_shared<CapsuleCollider>(CollisionCategory::PLAYER,CollisionCategory::ENEMY, Vector3{0.0f,0.0f,0.0f},1.0f,2.0f,Quaternion::identity);
	test2_ = std::make_shared<ConeCollider>(CollisionCategory::ENEMY,
		CollisionCategory::PLAYER | CollisionCategory::LIGHT,
		Vector3{ 0.0f,0.0f,0.0f },1.0f,2.0f,Quaternion::identity);

	test1Transform_ = std::make_unique<Transform>();
	test2Transform_ = std::make_unique<Transform>();

	test1_->SetParent(test1Transform_.get());
	test2_->SetParent(test2Transform_.get());

	collisionSystem_->RegisterCollider(test1_);
	collisionSystem_->RegisterCollider(test2_);

#pragma region Trolley
	trolley_ = std::make_unique<Trolley>();
	trolley_->Initialize();
#pragma endregion

#ifdef _DEBUG
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
#endif // _DEBUG

}

void GameScene::OnUpdate() {

#pragma region RailCameraSystem
	//現在のスピードを代入
	railCameraController_->SetPlaybackSpeed(trolley_->GetTrollySpeed());
	//更新
	railCameraController_->Update(1.0f / 60.0f);
	//現在のフレームのtransformを取得
	auto transform = railCameraController_->GetCurrentTransform();
	//座標変換
	transform = RailCameraSystem::RailCameraConverter::ConvertToLeftHand(transform);
	transform.UpdateMatrix();
	//カメラにセット
	camera_->SetPosition(transform.translate);
	camera_->SetRotate(transform.rotate);
	camera_->UpdateMatrices();

	RenderManager::GetInstance()->SetCamera(camera_);
#pragma endregion

#pragma region Flashlight
	flashlight_->Update();
#pragma endregion

#pragma region Trolley
	//カメラの座標をセット
	trolley_->SetTransform(transform);
	trolley_->Update();
#pragma endregion

#pragma region SceneObjectSystem
	sceneObjectManager_->Update();
#pragma endregion


#ifdef _DEBUG
	static bool isDebugCamera = false;
	ImGui::Begin("GameScene");
	//デバックカメラ
	if (ImGui::Checkbox("DebugCamera", &isDebugCamera)) {
		debugCamera_->SetTransform(transform);
	}
	if (isDebugCamera) {
		debugCamera_->Update();

		//線描画
		auto vertices = RailCameraSystem::RailCameraDebugUtils::CalculateFrustum(camera_->GetViewMatrix(), camera_->GetProjectionMatrix());

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

#ifdef _DEBUG
	ImGui::Begin("test");
	test1Transform_->Debug("test1");
	test2Transform_->Debug("test2");
	ImGui::End();
#endif

	collisionSystem_->CheckCollisions();

	const auto& collisions = test1_->GetCollidedWith();

	if (collisions.empty()) {
		//当たってない
	}
	else {
		// リストをループして、カテゴリごとに処理を分岐
		for (Collider* other : collisions) {

			switch (other->categoryBits) {
			case CollisionCategory::ENEMY:
				// (ここでプレイヤーのHPを減らす処理など)
				std::cout << "当たっています" << std::endl;
				break;
			default:
				break;
			}
		}
	}

}

void GameScene::OnFinalize() {

}
