#include "TitleScene.h"

#include "PersistentData.h"
#include "StageSelectScene.h"
#include "Graphics/RenderManager.h"
#include "Engine/Framework/AssetManager.h"
#include "Engine/File/JsonConverter.h"
#include "LightDeviceInput.h"
#include "GameSystem.h"

void TitleScene::OnInitialize() {
	auto assetManager = AssetManager::GetInstance();
	const auto& caveModel = assetManager->modelMap.Get("TitleStage")->Get();
	const auto& model = assetManager->modelMap.Get("TitleTrolley")->Get();

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
	title_.SetUVRect({ {0.0f, 0.0f}, {1.0f, 1.0f} }, Sprite::UVMode::UV);
	title_.SetAnchor({ 0.5f,0.5f });
	title_.SetPosition({ 1280.0f / 2.0f, 720.0f / 2.0f });
	title_.SetScale({ texture->GetSize()});

	collisionSystem_ = std::make_unique<CollisionSystem>();
	collisionSystem_->Initialize();

	flashlight_ = std::make_unique<Flashlight>();
	flashlight_->Initialize(&camera_->GetTransform(), camera_.get(), false);

	trolleyCollider_ = std::make_shared<SphereCollider>(
		CollisionCategory::DIORAMA,
		CollisionCategory::FLASHLIGHT,
		Vector3::zero,
		0.0f
	);
	trolleyCollider_->SetParent(&trolley_->transform);
	trolleyCollider_->radius = 3.0f;
	trolleyCollider_->center = Vector3{ -86.7f,1.0f,2.9f };

	collisionSystem_->RegisterCollider(flashlight_->GetCollider());
	collisionSystem_->RegisterCollider(trolleyCollider_);


	trolleyParticle_ = std::make_unique<ModelEmitter>();
	trolleyParticle_->Initialize(EmitShape::kSphere);
	trolleyParticle_->SetParent(&trolley_->transform);
	trolleyParticle_->SetRadius(3.0f);
	trolleyParticle_->SetOffset(Vector3{ -86.7f,0.0f,2.9f });

	RenderManager::GetInstance()->GetFogPostEffect().SetFogFactor(0.2f);
	isSceneChange_ = false;
	shakeTimer = 0.0f;
	chargeTimer = 0.0f;
	isRunning = false;

	const std::string texName[3] = { "MouseUsing","DeviceConnecting","DeviceUsing" };
	for (int i = 0; i < 3; ++i) {
		deviceUIs_[i] = std::make_unique<AnimeUI>();
		deviceUIs_[i]->sprite.SetTexture(assetManager->textureMap.Get(texName[i])->Get());
		deviceUIs_[i]->sprite.SetAnchor({ 0.0f, 1.0f });
		deviceUIs_[i]->sprite.SetColor({ 1.0f,1.0f,1.0f,0.4f });
		deviceUIs_[i]->sprite.SetPosition({ 0.0f, 720.0f + 60.0f });
		deviceUIs_[i]->sprite.SetScale({ 400.0f, 60.0f });
		deviceUIs_[i]->sprite.SetUVRect({ { 0.0f, 0.0f }, { 1.0f, 1.0f } }, Sprite::UVMode::UV);
		deviceUIs_[i]->sprite.SetIsActive(false);
		deviceUIs_[i]->timer = 0.0f;
		deviceUIs_[i]->play = false;
		deviceUIs_[i]->sprite.SetDrawOrder(0);
	}
	deviceState_ = 0;
	deviceUIs_[0]->sprite.SetPosition({ 0.0f, 720.0f });
	deviceUIs_[0]->sprite.SetIsActive(true);
	deviceUIs_[0]->timer = 1.0f;
	deviceUIs_[0]->sprite.SetDrawOrder(1);

	bgmAudioSource_ = assetManager->soundMap.Get("BGM_TITLE")->Get();
	bgmAudioSource_.Play(true);
	bgmAudioSource_.SetVolume(0.2f);

	textUI_ = std::make_unique<TextUI>();
	textUI_->Initialize({ 1280.0f * 0.5f,0.0f });

	circleGauge_ = std::make_unique<CircleGauge>();
	circleGauge_->Initialize(1.0f, { 376.0f,225.0f});

}

void TitleScene::OnUpdate() {
	camera_->UpdateMatrices();
	flashlight_->Update();
	collisionSystem_->CheckCollisions();
	trolleyParticle_->Update();
	textUI_->Update();

	circleGauge_->Update(chargeTimer);
	Vector3 shakeOffset = Vector3::zero; // 振動による位置のズレ

	// 1. 衝突判定（懐中電灯で照らされているか）
	bool isIlluminated = !trolleyCollider_->GetCollidedWith().empty();

	// 2. チャージと振動、およびパーティクル間隔の計算
	if (isRunning) {
		// 走り出している間は最大の振動とパーティクル頻度を維持
		shakeTimer += 0.8f;
		float intensity = 0.3f;
		shakeOffset.x = std::sin(shakeTimer * 1.1f) * intensity;
		shakeOffset.y = std::cos(shakeTimer * 0.9f) * intensity;
		shakeOffset.z = std::sin(shakeTimer * 1.5f) * intensity;

		// 走行中はパーティクルを最大頻度(1)で出し続ける
		trolleyParticle_->emitInterval_ = uint32_t(1.0f);
	}
	else if (isIlluminated) {
		// 照らされている間、チャージを蓄積
		chargeTimer = std::min(chargeTimer + 0.008f, 1.0f);
		shakeTimer += 0.6f;

		// パーティクル間隔を 10.0f から 1.0f へ近づける (線形補間)
		trolleyParticle_->emitInterval_ = uint32_t(10.0f + (1.0f - 10.0f) * chargeTimer);

		// チャージ量に応じて震えを大きくする (最大 0.3m)
		float intensity = chargeTimer * 0.3f;
		shakeOffset.x = std::sin(shakeTimer * 1.1f) * intensity;
		shakeOffset.y = std::cos(shakeTimer * 0.9f) * intensity;
		shakeOffset.z = std::sin(shakeTimer * 1.5f) * intensity;

		if (chargeTimer >= 1.0f) {
			isRunning = true;
			//俺が追加0.0fにすることで円ゲージを非表示にまずいかな？
			chargeTimer = 0.0f;
		}
	}
	else {
		// 照らしていない、かつ走り出していないならリセット
		chargeTimer = std::max(chargeTimer - 0.005f, 0.0f);
		shakeTimer = std::lerp(shakeTimer, 0.0f, 0.1f);
		shakeOffset = Vector3::Lerp(0.1f, shakeOffset, Vector3::zero);

		// 照らしていない時は間隔を戻す
		trolleyParticle_->emitInterval_ = uint32_t(20.0f);
	}

	RenderManager::GetInstance()->SetCamera(camera_);

	// --- トロッコ（Trolley）のアニメーション更新 ---
	const AnimationSet& anime = trolley_->animation->Get()->GetAnimation("\u5186\u67f1.002Action");
	auto it = anime.nodeAnimations.find("TitleTrollory");


	if (isRunning) {
		trolley_->animationTime += 0.016f / anime.duration;

		// ★アニメーションが終了（1.0以上）した時の処理
		if (trolley_->animationTime >= 1.0f) {
			trolley_->animationTime = 1.0f; // 1.0fに固定

			// ★シーン遷移の実行
			if (!isSceneChange_) {
				isSceneChange_ = true;
				SceneManager::GetInstance()->ChangeScene<StageSelectScene>(true);
			}
		}
	}
	else {
		// 走っていないときは 0 フレーム目で固定
		trolley_->animationTime = 0.0f;
	}

	if (it != anime.nodeAnimations.end()) {
		const NodeAnimation& nodeAnim = it->second;
		trolley_->transform.translate = CalculateValue(nodeAnim.translate, trolley_->animationTime);
		trolley_->transform.rotate = CalculateValue(nodeAnim.rotate, trolley_->animationTime);
		trolley_->transform.scale = CalculateValue(nodeAnim.scale, trolley_->animationTime);
	}

	// アニメーション位置に震え（オフセット）を加算
	trolley_->transform.translate = trolley_->transform.translate + shakeOffset;

	trolley_->transform.UpdateMatrix();
	trolley_->modelInstance.SetWorldMatrix(trolley_->transform.worldMatrix);

	// --- 背景（Cave）のアニメーション更新 ---
	const AnimationSet& caveAnime = cave_->animation->Get()->GetAnimation("CaveAction");
	auto caveIt = caveAnime.nodeAnimations.find("Cave");

	cave_->animationTime += 0.016f / caveAnime.duration;
	if (cave_->animationTime >= 1.0f) {
		cave_->animationTime = 0.0f;
	}

	if (caveIt != caveAnime.nodeAnimations.end()) {
		const NodeAnimation& nodeAnim = caveIt->second;
		cave_->transform.translate = CalculateValue(nodeAnim.translate, cave_->animationTime);
		cave_->transform.rotate = CalculateValue(nodeAnim.rotate, cave_->animationTime);
		cave_->transform.scale = CalculateValue(nodeAnim.scale, cave_->animationTime);
	}

	cave_->transform.UpdateMatrix();
	cave_->modelInstance.SetWorldMatrix(cave_->transform.worldMatrix);

	// --- デバッグ用などのシーン遷移（必要なければ削除可） ---
	// Input* input = Input::GetInstance();
	// if (input->IsKeyTrigger(DIK_SPACE)) {
	// 	SceneManager::GetInstance()->ChangeScene<StageSelectScene>(true);
	// }

	auto gameSystem = GameSystem::GetInstance();
	auto lightDeviceInput = LightDeviceInput::GetInstance();

	if (gameSystem->GetPlayDevice() != GameSystem::PlayDevice::LightDevice) {
		if (lightDeviceInput->GetConnectionState() == LightDeviceInput::ConnectionState::Disconnected) {
			if (input_->IsKeyPressed(DIK_LCONTROL) && input_->IsKeyTrigger(DIK_D)) {
				lightDeviceInput->Initialize();
			}
		}

		if (lightDeviceInput->GetConnectionState() == LightDeviceInput::ConnectionState::Connected) {
			gameSystem->SetPlayDevice(GameSystem::PlayDevice::LightDevice);
		}
	}

	int deviceStatePrev = deviceState_;

	if (lightDeviceInput->GetConnectionState() == LightDeviceInput::ConnectionState::Disconnected &&
		gameSystem->GetPlayDevice() == GameSystem::PlayDevice::KeyboardMouse) {
		deviceState_ = 0;
	}
	if (lightDeviceInput->GetConnectionState() == LightDeviceInput::ConnectionState::Connecting &&
		gameSystem->GetPlayDevice() == GameSystem::PlayDevice::KeyboardMouse) {
		deviceState_ = 1;
	}
	if (lightDeviceInput->GetConnectionState() == LightDeviceInput::ConnectionState::Connected &&
		gameSystem->GetPlayDevice() == GameSystem::PlayDevice::LightDevice) {
		deviceState_ = 2;
	}

	if (deviceState_ != deviceStatePrev) {
		deviceUIs_[deviceStatePrev]->play = true;
		deviceUIs_[deviceState_]->play = true;
		deviceUIs_[deviceState_]->sprite.SetIsActive(true);
		deviceUIs_[deviceState_]->sprite.SetDrawOrder(1);
		deviceUIs_[deviceStatePrev]->sprite.SetDrawOrder(0);
	}

	float speed = 1.0f / 60.0f;
	float waitPos = 60.0f;

	for (int i = 0; i < 3; ++i) {
		if (deviceUIs_[i]->play) {
			float t = 0;
			if (i == deviceState_) {
				deviceUIs_[i]->timer += speed;
				if (deviceUIs_[i]->timer >= 1.0f) {
					deviceUIs_[i]->timer = 1.0f;
					deviceUIs_[i]->play = false;
				}
				t = 1.0f - std::cos((1.0f - deviceUIs_[i]->timer) * Math::HalfPi);
			}
			else {
				deviceUIs_[i]->timer -= speed;
				if (deviceUIs_[i]->timer <= 0.0f) {
					deviceUIs_[i]->timer = 0.0f;
					deviceUIs_[i]->play = false;
					deviceUIs_[i]->sprite.SetIsActive(false);
				}
				t = std::sin((1.0f - deviceUIs_[i]->timer) * Math::HalfPi);
			}
			float y = t * waitPos + 720.0f;
			deviceUIs_[i]->sprite.SetPosition({ 0.0f, y });
		}
	}
}
void TitleScene::OnFinalize() {
	bgmAudioSource_.Stop();
}
