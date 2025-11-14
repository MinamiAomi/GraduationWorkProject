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

#include "RailCameraController.h"
#include "SceneObjectManager.h"

#include "Flashlight.h"
#include "Trolley.h"

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
	Input* input_;

	std::shared_ptr<Camera> camera_;

	std::unique_ptr<RailCameraSystem::RailCameraController> railCameraController_;

	std::unique_ptr<Flashlight> flashlight_;
	
	std::unique_ptr<SceneObjectSystem::SceneObjectManager> sceneObjectManager_;
	
	std::unique_ptr<Trolley> trolley_;
};