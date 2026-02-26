#pragma once
#include "Scene/BaseScene.h"

#include <memory>

#include "DebugCamera.h"
#include "Engine/Input/Input.h"
#include "Scene/SceneManager.h"
#include "Graphics/Sprite.h"
#include "Flashlight.h"
#include "Diorama.h"
#include "Engine/Graphics/Animation.h"
#include "Framework/AssetManager.h"
#include "Graphics/LightManager.h"
#include "Audio/AudioSource.h"
#include "Utility/Utility.h"

#include "CollisionSystem.h"
#include "AnimationTransform.h"
#include "ClearParticles.h"

class GameClearScene :
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

	std::unique_ptr<Diorama> selectTriangleLeft_;
	std::unique_ptr<Diorama> selectTriangleRight_;
	Transform parentTransform_;

    ModelInstance trolleyModelInstance_;
    AnimationTransform trolleyAnimationTransform_;

    ModelInstance oodamaModelInstance_;
    AnimationTransform oodamaAnimationTransform_;

    ModelInstance stageModelInstance_;
	
	std::unique_ptr<Flashlight> flashlight_;
	std::unique_ptr<CollisionSystem> collisionSystem_;
	AudioSource seAudioSource_;
	Sprite sprite_;

    Utility::Timer timer_;

	std::unique_ptr<ClearParticles> clearParticles_;

	bool isSceneChange_;
};