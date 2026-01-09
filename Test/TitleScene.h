#pragma once
#include "Scene/BaseScene.h"

#include "Engine/Input/Input.h"
#include "Scene/SceneManager.h"
#include "Graphics/Sprite.h"
#include "Math/Camera.h"
#include "Math/Transform.h"
#include "Graphics/Model.h"
#include "DeviceOptionsUI.h"
#include "ModelEmitter.h"


class TitleScene :
	public BaseScene {
public:

	static const uint32_t stoneNum = 15;

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
	std::unique_ptr<ModelInstance> stoneModels_[stoneNum];
	std::unique_ptr<Transform> stoneTransforms_[stoneNum];
	Vector3 stonePositions_[stoneNum];

	std::unique_ptr<ModelEmitter> modelEmitter_;
	Vector3 testPos_;
	Quaternion testQuatenion_;
    std::unique_ptr<DeviceOptionsUI> deviceOptionsUI_;
};