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

	auto result = SceneObjectSystem::SceneLoader::LoadSceneFromFile("Resources/StaticMesh/staticMesh.json");

	sceneObjectManager_->CreateObjects(result);
#pragma endregion
	collisionSystem_ = std::make_unique<CollisionSystem>();
	test1_ = std::make_shared<SphereCollider>(CollisionCategory::PLAYER,static_cast<uint32_t>(CollisionCategory::ENEMY), Vector3{0.0f,0.0f,0.0f},1.0f);
	test2_ = std::make_shared<BoxCollider>(CollisionCategory::ENEMY,
		static_cast<uint32_t>(CollisionCategory::PLAYER | CollisionCategory::LIGHT),
		Vector3{ 1.0f,0.0f,0.0f }, Vector3{1.0f,1.0f,1.0f});
	collisionSystem_->RegisterCollider(test1_);
	collisionSystem_->RegisterCollider(test2_);

}

void GameScene::OnUpdate() {

#pragma region RailCameraSystem
	railCameraController_->Update(1.0f / 60.0f);
	auto transform = railCameraController_->GetCurrentTransform();
	transform = RailCameraSystem::RailCameraConverter::ConvertToLeftHand(transform);
	transform.UpdateMatrix();

	camera_->SetPosition(transform.translate);
	camera_->SetRotate(transform.rotate);
	camera_->UpdateMatrices();
	//カメラのデバック用
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
#pragma endregion

#pragma region Flashlight
	flashlight_->Update();
#pragma endregion

#pragma region SceneObjectSystem
	sceneObjectManager_->Update();
#pragma endregion

	//ここコメントアウトすればデバックカメラ使用可能
	RenderManager::GetInstance()->SetCamera(camera_);

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
