#pragma once

#include <memory>

#include "Scene/BaseScene.h"

#include "DebugCamera.h"
#include "Engine/Input/Input.h"
#include "Scene/SceneManager.h"
#include "Graphics/Sprite.h"
#include "Flashlight.h"
#include "Diorama.h"
#include "Audio/AudioSource.h"

#include "CollisionSystem.h"

class StageSelectScene :
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
	Input* input_;
	std::shared_ptr<DebugCamera> camera_;

	std::unique_ptr<Diorama> level1_;
	std::unique_ptr<Diorama> level2_;
	std::unique_ptr<Flashlight> flashlight_;
	std::unique_ptr<CollisionSystem> collisionSystem_;
	ModelInstance iceSkyDome_;
	ModelInstance stageSelectTerrain_;
	Sprite sprite_;

    AudioSource bgmAudioSource_;

	bool isSceneChange_;

#ifdef _DEBUG
	Transform modelTransform_;
	ModelInstance model_;
#endif // DEBUG

};