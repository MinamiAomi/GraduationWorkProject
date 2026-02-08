#pragma once
#include "Scene/BaseScene.h"

#include "DebugCamera.h"
#include "Engine/Input/Input.h"
#include "Scene/SceneManager.h"
#include "Graphics/Sprite.h"

#include "Diorama.h"

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

	Diorama level1_;
	Diorama level2_;
	//Sprite sprite_;
};