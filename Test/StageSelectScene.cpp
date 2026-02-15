#include "StageSelectScene.h"

#include "PersistentData.h"
#include "GameScene.h"
#include "LevelManager.h"

#include "Framework/AssetManager.h"

#include "TitleScene.h"

void StageSelectScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	input_ = Input::GetInstance();

	camera_ = std::make_shared<DebugCamera>();
	camera_->Initialize();
	RenderManager::GetInstance()->SetCamera(camera_->GetCamera());

	RenderManager::GetInstance()->GetFogPostEffect().SetFogFactor(0.2f);
	
	collisionSystem_ = std::make_unique<CollisionSystem>();

	if (persistentData_) {
		persistentData_->score_->num = 0;
	}

	auto texture = AssetManager::GetInstance()->textureMap.Get("StageSelect")->Get();
	auto iceSkyDome = AssetManager::GetInstance()->modelMap.Get("IceSkyDome")->Get();
	auto stageSelectTerrain = AssetManager::GetInstance()->modelMap.Get("StageSelectTerrain")->Get();

	sprite_.SetTexture(texture);
	sprite_.SetUVRect({ { 0.0f, 0.0f }, { 1.0f, 1.0f} }, Sprite::UVMode::UV);
	sprite_.SetPosition({ 1280.0f * 0.5f,720.0f * 0.5f });
	sprite_.SetScale({ texture->GetSize() });

	level1_ = std::make_unique<Diorama>();
	level2_ = std::make_unique<Diorama>();

	level1_->Initialize("DioramaLevel1", Vector3(-2.3f, -1.0f, 1.0f));
	level2_->Initialize("DioramaLevel2", Vector3(2.3f, -1.0f, 1.0f));
	
	collisionSystem_->RegisterCollider(level1_->GetCollider());
	collisionSystem_->RegisterCollider(level2_->GetCollider());

	iceSkyDome_.SetModel(iceSkyDome);
	stageSelectTerrain_.SetModel(stageSelectTerrain);

	flashlight_ = std::make_unique<Flashlight>();
	//flashlight_->SetRailAnimationPlayer(railAnimationPlayer_.get());
	flashlight_->Initialize(&camera_->GetCamera()->GetTransform(), camera_->GetCamera().get());
	collisionSystem_->RegisterCollider(flashlight_->GetCollider());

	bgmAudioSource_ = AssetManager::GetInstance()->soundMap.Get("BGM_STAGE_SELECT")->Get();
	bgmAudioSource_.Play(true);
	bgmAudioSource_.SetVolume(0.2f);
}

void StageSelectScene::OnUpdate() {

	camera_->Update();

	flashlight_->Update();
	
	level1_->Update();
	level2_->Update();

	if (level1_->GetIsActive()) {
		LevelManager::GetInstance()->SetLevel(LevelManager::Level::LEVEL1);
		SceneManager::GetInstance()->ChangeScene<GameScene>(false);
	}
	else if (level2_->GetIsActive()) {
		LevelManager::GetInstance()->SetLevel(LevelManager::Level::LEVEL2);
		SceneManager::GetInstance()->ChangeScene<GameScene>(false);
	}

	collisionSystem_->CheckCollisions();

	Input* input = Input::GetInstance();
	if (input->IsKeyTrigger(DIK_ESCAPE)) {
		// ゲームスタート
		SceneManager::GetInstance()->ChangeScene<TitleScene>(true);
	}
//#ifdef _DEBUG
//	ImGui::Begin("StageModel");
//	
//	ipos = iceSkyDome_.GetWorldMatrix().GetTranslate();
//	iscale= iceSkyDome_.GetWorldMatrix().GetScale();
//	ImGui::DragFloat3("IcePos", &ipos.x);
//	ImGui::DragFloat3("IceScale", &iscale.x);
//	iceSkyDome_.SetWorldMatrix(Matrix4x4::MakeAffineTransform(ipos, Quaternion::identity, iscale));
//
//	spos = stageSelectTerrain_.GetWorldMatrix().GetTranslate();
//	sscale= stageSelectTerrain_.GetWorldMatrix().GetScale();
//	ImGui::DragFloat3("StagePos", &spos.x);
//	ImGui::DragFloat3("StageScale", &sscale.x);
//	stageSelectTerrain_.SetWorldMatrix(Matrix4x4::MakeAffineTransform(spos, Quaternion::identity, sscale));
//
//	ImGui::End();
//#endif // _DEBUG

}

void StageSelectScene::OnFinalize() {
	if (bgmAudioSource_.IsPlaying()){
		bgmAudioSource_.Stop();
	}
}
