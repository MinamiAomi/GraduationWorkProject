#pragma once
#include "Scene/BaseScene.h"

#include <memory>

#include "Engine/Input/Input.h"
#include "Scene/SceneManager.h"
#include "Math/Camera.h"
#include "Math/Transform.h"
#include "Math/Random.h"
#include "Graphics/Model.h"
#include "Graphics/LightManager.h"
#include "Audio/AudioSource.h"
#include "Graphics/Skeleton.h"
#include "Graphics/Sprite.h"

#include "RailAnimationPlayer.h"
#include "RailCameraSystem.h"
#include "SceneObjectManager.h"

#include "Flashlight.h"
#include "Trolley.h"
#include "Deadline.h"
#include "BatsManager.h"
#include "TutorialObject.h"


#include "RailcameraUI.h"

#ifdef _DEBUG
#include "DebugCamera.h"

#include "Graphics/Model.h"
#endif // _DEBUG

#include "Collider.h"
#include "CollisionSystem.h"
#include "BatteryParticles.h"

class GameScene :
	public BaseScene {
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void OnInitialize() override;
	/// <summary>
	/// 更新
	/// </summary>
	void OnUpdate() override;
	/// <summary>
	/// 終了処理
	/// </summary>
	void OnFinalize() override;

private:
	const uint32_t kDirectionalLightCount = 6;

	Input* input_;

	std::shared_ptr<Camera> camera_;

	std::vector<std::shared_ptr<DirectionalLight>> directionalLights_;

	std::unique_ptr<RailSystem::RailAnimationPlayer> railAnimationPlayer_;
	std::unique_ptr<RailSystem::RailCameraSystem> railCameraSystem_;
	std::unique_ptr<Deadline> deadline_;

	std::unique_ptr<Flashlight> flashlight_;
	
	std::unique_ptr<SceneObjectSystem::SceneObjectManager> sceneObjectManager_;
	
	Trolley* trolley_;
	std::unique_ptr<BatteryParticles> batteryParticles_;

	std::unique_ptr<CollisionSystem> collisionSystem_;
	std::unique_ptr<BatsManager> batsManager_;
	std::unique_ptr<RailcameraUI> railcameraUI_;

	std::unique_ptr<TutorialObject> trollyTutorial_;
	std::unique_ptr<TutorialObject> flashlightTutorial_;

	Sprite inGameUI_;
	int inGameUICount_;
	int inGameUIMaxCount_;

	Sprite crackUI_;
	Sprite gameFinishBackGround_;
	int gameFinishCount_;
	int gameFinishMaxCount_;
	//クリア化ゲームオーバー化
	bool isClear_;

	//クリアorゲームオーバーしてアニメーションをスタートするフラグ
	bool isGameFinishAnimation_;
	//クリアorゲームオーバーアニメーションが終了したかどうかのフラグ
	bool isGameFinalizeAnimation_;

	std::unique_ptr<AudioSource> bgmAudioSource_;

	bool isPlay_ = true;

#ifdef _DEBUG
	std::unique_ptr<DebugCamera> debugCamera_;
#endif // _DEBUG


};