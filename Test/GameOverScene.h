#pragma once
#include "Scene/BaseScene.h"

#include "DebugCamera.h"
#include "Engine/Input/Input.h"
#include "Scene/SceneManager.h"
#include "Graphics/Sprite.h"
#include "Flashlight.h"
#include "Diorama.h"

#include "CollisionSystem.h"

class GameOverScene :
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

	std::unique_ptr<Diorama> selectTriangleLeft_;
	std::unique_ptr<Diorama> selectTriangleRight_;
	std::unique_ptr<Flashlight> flashlight_;
	std::unique_ptr<CollisionSystem> collisionSystem_;
	Sprite sprite_;
};