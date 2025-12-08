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
#pragma region Deadline
	deadline_ = std::make_unique<Deadline>();
	deadline_->SetAnimationPlayer(railAnimationPlayer_.get());
	deadline_->Initialize();
#pragma endregion


#ifdef _DEBUG
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
#endif // _DEBUG

}

void GameScene::OnUpdate() {
	float deltaTime = 1.0f / 60.0f;

#ifdef DEBUG
	if (deadline_->IsGameOver()) {
		return;
	}

	//一周終わったかどうか
	if (railAnimationPlayer_->IsFinished()) {
		return;
	}
#endif // _DEBUG

#pragma region RailSystem

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
#pragma region Deadline
	deadline_->Update(deltaTime);
#pragma endregion


#pragma region CollisionSystem
	collisionSystem_->CheckCollisions();
#pragma endregion
#ifdef _DEBUG

	ImGui::Begin("RailAnimationPlayer");
	if (railAnimationPlayer_->IsPlaying()) {
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: Playing >>");
	}
	else {
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Status: Paused ||");
	}

	//区切り線
	ImGui::Separator();

	ImGui::Text("現実速度（傾きやFOVはこの速度参照）:%.2f", railAnimationPlayer_->GetRealSpeed());
	// 余白
	ImGui::Spacing();

	float minFrame = static_cast<float>(railAnimationPlayer_->GetRailAnimationDate()->railCameraMetaData_.startFrame);
	float maxFrame = static_cast<float>(railAnimationPlayer_->GetRailAnimationDate()->railCameraMetaData_.endFrame);

	// スライダーで直感的に位置を変更・確認できるようにする
	ImGui::Text("Timeline");
	float currentFrame = railAnimationPlayer_->GetCurrentFrame();
	ImGui::SliderFloat("##FrameSlider", &currentFrame, minFrame, maxFrame, "Frame: %.2f");
	ImGui::InputFloat("##Frame", &currentFrame);
	currentFrame = std::clamp(currentFrame, float(railAnimationPlayer_->GetRailAnimationDate()->railCameraMetaData_.startFrame), float(railAnimationPlayer_->GetRailAnimationDate()->railCameraMetaData_.endFrame));
	railAnimationPlayer_->SetCurrentFrame(currentFrame);

	// 進捗バー
	float progress = (currentFrame - minFrame) / (maxFrame - minFrame);
	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));

	// 余白
	ImGui::Spacing();


	// 再生ボタン
	if (ImGui::Button("Play")) {
		railAnimationPlayer_->Play();
	}
	//横並びに
	ImGui::SameLine();

	// 一時停止ボタン
	if (ImGui::Button("Pause")) {
		railAnimationPlayer_->Pause();
	}
	//横並びに
	ImGui::SameLine();

	// 停止
	if (ImGui::Button("Reset")) {
		railAnimationPlayer_->Loop();
		flashlight_->Initialize(&camera_->GetTransform(), camera_.get());
		trolley_->Initialize();
		railCameraSystem_->Initialize();
		deadline_->Initialize();

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
	}

	ImGui::Separator();

	if (ImGui::TreeNode("Json情報")) {
		ImGui::Text("最初のフレーム: %d", railAnimationPlayer_->GetRailAnimationDate()->railCameraMetaData_.startFrame);
		ImGui::Text("最後のフレーム : %d", railAnimationPlayer_->GetRailAnimationDate()->railCameraMetaData_.endFrame);
		ImGui::Text("フレームレート : %d", railAnimationPlayer_->GetRailAnimationDate()->railCameraMetaData_.frameRate);

		ImGui::TreePop();
	}
	ImGui::End();

	//一周終わったかどうか
	if (railAnimationPlayer_->IsFinished()) {
		flashlight_->Initialize(&camera_->GetTransform(), camera_.get());
		trolley_->Initialize();
		railCameraSystem_->Initialize();
		deadline_->Initialize();

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

	//if (input_->IsKeyTrigger(DIK_SPACE)) {
	//	SceneManager::GetInstance()->ChangeScene<GameOverScene>();
	//}


#endif

#ifdef DEBUG


	//ゲームオーバー
	if (deadline_->IsGameOver()) {
		SceneManager::GetInstance()->ChangeScene<GameOverScene>();
	}

	//一周終わったかどうか
	if (railAnimationPlayer_->IsFinished()) {
		SceneManager::GetInstance()->ChangeScene<GameClearScene>();
	}
#endif // DEBUG
}

void GameScene::OnFinalize() {

}
