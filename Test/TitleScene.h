#pragma once
#include "Scene/BaseScene.h"

#include "Engine/Input/Input.h"
#include "Scene/SceneManager.h"
#include "Graphics/Sprite.h"
#include "DebugCamera.h"
#include "Math/Transform.h"
#include "Graphics/Model.h"
#include "DeviceOptionsUI.h"
#include "Bats.h"
#include "Engine/Graphics/Animation.h"
#include "CollisionSystem.h"
#include "Flashlight.h"
#include "ModelEmitter.h"
#include "Audio/AudioSource.h"


class TitleScene :
	public BaseScene {
public:

	struct AnimationModel {
		ModelInstance modelInstance;
		Transform transform;
		std::shared_ptr<AnimationAsset> animation;
		float animationTime;
	};

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
	Input* input_;
	Sprite title_;
	std::shared_ptr<Camera> camera_;
	//std::unique_ptr<DeviceOptionsUI> deviceOptionsUI_;
	std::unique_ptr<AnimationModel> trolley_;
	std::unique_ptr<AnimationModel> cave_;
	std::unique_ptr<Flashlight> flashlight_;
	std::unique_ptr<CollisionSystem> collisionSystem_;
	std::shared_ptr<SphereCollider> trolleyCollider_;

	AudioSource bgmAudioSource_;
	std::unique_ptr<ModelEmitter> trolleyParticle_;
	bool isSceneChange_ = false;
	float shakeTimer = 0.0f;
	float chargeTimer = 0.0f;
	bool isRunning = false;
};