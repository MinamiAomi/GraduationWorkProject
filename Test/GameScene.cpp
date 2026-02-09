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

#include "AnimationLoader.h"

#include "LevelManager.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

void GameScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	input_ = Input::GetInstance();

	camera_ = std::make_shared<Camera>();
	auto currentLevel = LevelManager::GetInstance()->GetLevel();
	std::string railcameraJson, staticMeshJson, stageName;
	switch (currentLevel)
	{
	case LevelManager::Level::LEVEL1:
		railcameraJson = "Resources/RailCamera/Level1_railCamera.json";
		staticMeshJson = "Resources/StaticMesh/Level1_StaticMesh.json";
		stageName = "Stage1";
		break;
	case LevelManager::Level::LEVEL2:
		railcameraJson = "Resources/RailCamera/Level2_railCamera.json";
		staticMeshJson = "Resources/StaticMesh/Level2_StaticMesh.json";
		stageName = "Stage2";
		break;
	default:
		break;
	}
#pragma region CollisionSystem
	collisionSystem_ = std::make_unique<CollisionSystem>();
#pragma endregion

	directionalLights_.resize(kDirectionalLightCount);
	for (uint32_t i = 0; i < kDirectionalLightCount; ++i) {
		auto& directionalLight = directionalLights_[i];
		directionalLight = std::make_shared<DirectionalLight>();
		directionalLight->color = Color(1.0f, 1.0f, 1.0f);
		float t = Math::TwoPi * ((float)i / (float)kDirectionalLightCount);
		directionalLight->direction = { std::cos(t), -1.0f , std::sin(t) };
		directionalLight->intensity = 0.2f;
		directionalLight->isActive = true;
		RenderManager::GetInstance()->GetLightManager().Add(directionalLight);
	}

#pragma region RailSystem
	auto animationData = AnimationUtils::AnimationLoader::LoadRailAnimation(railcameraJson);
	if (animationData) {
		railAnimationPlayer_ = std::make_unique<RailSystem::RailAnimationPlayer>
			(
				std::make_shared<const RailSystem::RailAnimation>(*animationData)
			);
		//カメラ再生
		railAnimationPlayer_->Play();
	}
	else {
		assert(0);
	}

#pragma endregion

#pragma region Flashlight
	flashlight_ = std::make_unique<Flashlight>();
	flashlight_->SetRailAnimationPlayer(railAnimationPlayer_.get());
	flashlight_->Initialize(&camera_->GetTransform(), camera_.get());
	collisionSystem_->RegisterCollider(flashlight_->GetCollider());
#pragma endregion

#pragma region Trolley
	trolley_ = Trolley::GetInstance();
	trolley_->SetIsActive(true);
	trolley_->SetParent(railAnimationPlayer_->GetTransform());
	trolley_->SetRailAnimationPlayer(railAnimationPlayer_.get());
	trolley_->SetFlashlight(flashlight_.get());
	trolley_->Initialize();
	for (auto& collider : trolley_->GetColliders()) {
		collisionSystem_->RegisterCollider(collider);
	}
	batteryParticles_ = std::make_unique<BatteryParticles>();
	batteryParticles_->Initialize(&trolley_->GetBatteyTransform(0), batsManager_.get());
#pragma endregion

#pragma region RailCameraSystem
	railCameraSystem_ = std::make_unique<RailSystem::RailCameraSystem>();
	railCameraSystem_->SetRailAnimationPlayer(railAnimationPlayer_.get());
	railCameraSystem_->SetParent(trolley_->GetTransform());
	railCameraSystem_->Initialize();
	trolley_->SetBatteyParent(railCameraSystem_->GetTransform());
#pragma endregion


#pragma region SceneObjectSystem

	sceneObjectManager_ = std::make_unique<SceneObjectSystem::SceneObjectManager>();

	sceneObjectManager_->Initialize();

	auto result = SceneObjectSystem::SceneLoader::LoadSceneFromFile(staticMeshJson);

	sceneObjectManager_->CreateObjects(result, stageName);

	//Colliderセット
	for (const auto& collider : sceneObjectManager_->GetPointLightObjects()) {
		collisionSystem_->RegisterCollider(collider->collider);
	}
	for (const auto& collider : sceneObjectManager_->GetEnemySpawnObjects()) {
		collisionSystem_->RegisterCollider(collider->collider);
	}
	for (const auto& collider : sceneObjectManager_->GetGimmickTriggerObjects()) {
		collisionSystem_->RegisterCollider(collider->collider);
	}
	for (const auto& collider : sceneObjectManager_->GetGimmickPointLightObjects()) {
		collisionSystem_->RegisterCollider(collider->collider);
	}
	for (const auto& collider : sceneObjectManager_->GetObstacleObjects()) {
		collisionSystem_->RegisterCollider(collider->collider);
	}
#pragma endregion
#pragma region Deadline
	deadline_ = std::make_unique<Deadline>();
	deadline_->SetAnimationPlayer(railAnimationPlayer_.get());
	deadline_->Initialize();
#pragma endregion
#pragma region Bats
	batsManager_ = std::make_unique<BatsManager>();
	batsManager_->SetCamera(camera_.get());
	batsManager_->SetColliderSystem(collisionSystem_.get());
	//test
	std::vector<std::vector<bool>> mapData(5, std::vector<bool>(6, false));

	// 2. 真ん中の要素を true にする
	// 行: 5の中央は 2
	// 列: 6の中央は 2 または 3 (ここでは2を選択)
	mapData[2][2] = true;

	// 3. 実行
	batsManager_->Emit(mapData);

	batteryParticles_->Initialize(&trolley_->GetBatteyTransform(0), batsManager_.get());
	sceneObjectManager_->SetBatsManager(batsManager_.get());
#pragma endregion
#pragma region RailcameraUI
	railcameraUI_ = std::make_unique<RailcameraUI>();
	railcameraUI_->Initialize();
#pragma endregion
#pragma region TutorialObject
	switch (currentLevel)
	{
	case LevelManager::Level::LEVEL1:
	{
		trollyTutorial_ = std::make_unique<TutorialObject>();
		flashlightTutorial_ = std::make_unique<TutorialObject>();
		Transform t;
		t.translate = { 17.0f,1.7f,4.0f };
		trollyTutorial_->Initialize(t, "TutorialTrolly");
		collisionSystem_->RegisterCollider(trollyTutorial_->GetCollider());
		t.translate = { 0.0f,0.0f,0.0f};
		flashlightTutorial_->Initialize(t, "TutorialFlashlight");
		collisionSystem_->RegisterCollider(flashlightTutorial_->GetCollider());
	}
	break;
	case LevelManager::Level::LEVEL2:
		break;
	default:
		break;
	}
#pragma endregion


#ifdef _DEBUG
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
#endif // _DEBUG

}

void GameScene::OnUpdate() {
	float deltaTime = 1.0f / 60.0f;
	auto currentLevel = LevelManager::GetInstance()->GetLevel();
#ifdef _DEBUG
	if (deadline_->IsGameOver()) {
		return;
	}

	//一周終わったかどうか
	if (railAnimationPlayer_->IsFinished()) {
		return;
	}

#endif // _DEBUG

#pragma region TutorialObject

	switch (currentLevel)
	{
	case LevelManager::Level::LEVEL1:
		trollyTutorial_->Update();
		flashlightTutorial_->Update();
		if (trollyTutorial_->GetIsActive() || flashlightTutorial_->GetIsActive()) {
			railAnimationPlayer_->Pause();
			flashlight_->Pause();
			trolley_->Pause();
			isPlay_ = false;
		}
		else if (!isPlay_) {
			flashlight_->Play();
			trolley_->Play();
			railAnimationPlayer_->Play();
			isPlay_ = true;
		}
		break;
	case LevelManager::Level::LEVEL2:
		break;
	default:
		break;
	}
#pragma endregion
#pragma region RailSystem

	//現在のスピードを代入
	railAnimationPlayer_->SetPlaybackSpeed(trolley_->GetTrollySpeed());
	//更新
	railAnimationPlayer_->Update(deltaTime);



#pragma endregion

#pragma region RailCameraSystem
	railCameraSystem_->Update(deltaTime);
	//camera_->SetFov(railCameraSystem_->GetFov());
	camera_->SetRotate(railCameraSystem_->GetWorldRotation());
	camera_->SetPosition(railCameraSystem_->GetWorldTranslate());
#pragma endregion

	//カメラ処理終わりこの後は動きません
	camera_->UpdateMatrices();
	RenderManager::GetInstance()->SetCamera(camera_);


#pragma region Trolley
	trolley_->Update(deltaTime);
#pragma endregion

#pragma region Flashlight
	flashlight_->Update();
#pragma endregion

#pragma region Flashlight
	flashlight_->Update();
#pragma endregion
#pragma region Bats
	batsManager_->SetCamera(camera_.get());
	batsManager_->Update();
#pragma endregion
#pragma region RailcameraUI
	railcameraUI_->Update(
		(railAnimationPlayer_->GetCurrentFrame() / railAnimationPlayer_->GetRailAnimationDate()->railMetaData_.endFrame),
		(deadline_->GetCurrenFrame() / railAnimationPlayer_->GetRailAnimationDate()->railMetaData_.endFrame)
	);
#pragma endregion

	batteryParticles_->Update();

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
	PowerEmitter::Debug();
	batteryParticles_->Debug();
	batteryParticles_->DebugDraw();
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

	float minFrame = static_cast<float>(railAnimationPlayer_->GetRailAnimationDate()->railMetaData_.startFrame);
	float maxFrame = static_cast<float>(railAnimationPlayer_->GetRailAnimationDate()->railMetaData_.endFrame);

	// スライダーで直感的に位置を変更・確認できるようにする
	ImGui::Text("Timeline");
	float currentFrame = railAnimationPlayer_->GetCurrentFrame();
	ImGui::SliderFloat("##FrameSlider", &currentFrame, minFrame, maxFrame, "Frame: %.2f");
	ImGui::InputFloat("##Frame", &currentFrame);
	currentFrame = std::clamp(currentFrame, float(railAnimationPlayer_->GetRailAnimationDate()->railMetaData_.startFrame), float(railAnimationPlayer_->GetRailAnimationDate()->railMetaData_.endFrame));
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
		deadline_->Initialize();

		trolley_->SetParent(railAnimationPlayer_->GetTransform());
		trolley_->SetRailAnimationPlayer(railAnimationPlayer_.get());
		trolley_->SetFlashlight(flashlight_.get());
		trolley_->Initialize();
		batteryParticles_->Initialize(&trolley_->GetBatteyTransform(0), batsManager_.get());

		railCameraSystem_->SetRailAnimationPlayer(railAnimationPlayer_.get());
		railCameraSystem_->SetParent(trolley_->GetTransform());
		railCameraSystem_->Initialize();
		trolley_->SetBatteyParent(railCameraSystem_->GetTransform());



		//SceneObjectsリセット
		sceneObjectManager_->ResetObjects();


		//Colliderセット
		for (const auto& collider : sceneObjectManager_->GetPointLightObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
		for (const auto& collider : sceneObjectManager_->GetEnemySpawnObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
		for (const auto& collider : sceneObjectManager_->GetGimmickTriggerObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
		for (const auto& collider : sceneObjectManager_->GetGimmickPointLightObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
		for (const auto& collider : sceneObjectManager_->GetObstacleObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
	}

	ImGui::Separator();

	if (ImGui::TreeNode("Json情報")) {
		ImGui::Text("最初のフレーム: %d", railAnimationPlayer_->GetRailAnimationDate()->railMetaData_.startFrame);
		ImGui::Text("最後のフレーム : %d", railAnimationPlayer_->GetRailAnimationDate()->railMetaData_.endFrame);
		ImGui::Text("フレームレート : %d", railAnimationPlayer_->GetRailAnimationDate()->railMetaData_.frameRate);

		ImGui::TreePop();
	}
	ImGui::End();

	//一周終わったかどうか
	//if (railAnimationPlayer_->IsFinished()) {
	//	flashlight_->Initialize(&railCameraSystem_->GetTransform(), camera_.get());
	//	trolley_->Initialize();
	//	railCameraSystem_->Initialize();
	//	deadline_->Initialize();

	//	//SceneObjectsリセット
	//	sceneObjectManager_->ResetObjects();

	//	//Colliderセット
	//	for (const auto& collider : sceneObjectManager_->GetPointLightObjects()) {
	//		collisionSystem_->RegisterCollider(collider->collider);
	//	}
	//	for (const auto& collider : sceneObjectManager_->GetEnemySpawnObjects()) {
	//		collisionSystem_->RegisterCollider(collider->collider);
	//	}
	//	for (const auto& collider : sceneObjectManager_->GetGimmickTriggerObjects()) {
	//		collisionSystem_->RegisterCollider(collider->collider);
	//	}
	//	for (const auto& collider : sceneObjectManager_->GetGimmickPointLightObjects()) {
	//		collisionSystem_->RegisterCollider(collider->collider);
	//	}
	//	for (const auto& collider : sceneObjectManager_->GetObstacleObjects()) {
	//		collisionSystem_->RegisterCollider(collider->collider);
	//	}
	//	railAnimationPlayer_->Loop();
	//}
	static bool isDebugCamera = false;
	ImGui::Begin("GameScene");
	for (uint32_t i = 0; i < kDirectionalLightCount; ++i) {
		auto& directionalLight = directionalLights_[i];
		directionalLight->DrawImGui(std::format("DirectionalLight{}", i));
	}

	if (ImGui::Button("ホットリロード（光物）")) {
		sceneObjectManager_->Initialize();

		std::string railcameraJson, staticMeshJson, stageName;
		switch (currentLevel)
		{
		case LevelManager::Level::LEVEL1:
			railcameraJson = "Resources/RailCamera/Level1_railCamera.json";
			staticMeshJson = "Resources/StaticMesh/Level1_StaticMesh.json";
			stageName = "Stage1";
			break;
		case LevelManager::Level::LEVEL2:
			railcameraJson = "Resources/RailCamera/Level2_railCamera.json";
			staticMeshJson = "Resources/StaticMesh/Level2_StaticMesh.json";
			stageName = "Stage2";
			break;
		default:
			break;
		}

		auto result = SceneObjectSystem::SceneLoader::LoadSceneFromFile(staticMeshJson);

		sceneObjectManager_->CreateObjects(result, stageName);

		//Colliderセット
		for (const auto& collider : sceneObjectManager_->GetPointLightObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
		for (const auto& collider : sceneObjectManager_->GetEnemySpawnObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
		for (const auto& collider : sceneObjectManager_->GetGimmickTriggerObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
		for (const auto& collider : sceneObjectManager_->GetGimmickPointLightObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
		for (const auto& collider : sceneObjectManager_->GetObstacleObjects()) {
			collisionSystem_->RegisterCollider(collider->collider);
		}
	}

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
		SceneManager::GetInstance()->ChangeScene<GameOverScene>();
	}
#endif

	//ゲームオーバー
	if (deadline_->IsGameOver()) {
		SceneManager::GetInstance()->ChangeScene<GameOverScene>();
	}

	//一周終わったかどうか
	if (railAnimationPlayer_->IsFinished()) {
		SceneManager::GetInstance()->ChangeScene<GameClearScene>();
	}
}

void GameScene::OnFinalize() {
	trolley_->SetIsActive(false);
}
