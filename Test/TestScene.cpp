#include "TestScene.h"

#include "Framework/Engine.h"
#include "Input/Input.h"
#include "Graphics/RenderManager.h"
#include "Framework/AssetManager.h"
#include "Graphics/Core/TextureLoader.h"
#include "LevelLoader.h"

#include "RailCameraLoader.h"
#include "RailCameraConverter.h"
#include "RailCameraDebugUtils.h"

void TestScene::OnInitialize() {

	sunLight_ = std::make_shared<DirectionalLight>();
	sunLight_->direction = -Vector3::unitY;
	RenderManager::GetInstance()->SetSunLight(sunLight_);


	const Vector3 kOffset = { 50.0f, 20.0f, 0.0f };
	const float kWidth = 3.0f;
	const Vector3 kSize = { kColumnCount * kWidth, kRowCount * kWidth, 0.0f };

	auto sphereModel = AssetManager::GetInstance()->modelMap.Get("sphere");
	for (uint32_t row = 0; row < kRowCount; ++row) {
		for (uint32_t column = 0; column < kColumnCount; ++column) {
			auto& sphere = spheres_[row][column];
			Vector3 position = Vector3{ column * kWidth, row * kWidth, 0.0f } - (kSize * 0.5f) + kOffset;
			sphere.material = std::make_shared<Material>();
			sphere.material->albedo = { 1.0f, 1.0f, 1.0f };
			sphere.material->metallic = (float)row / (kRowCount - 1);
			sphere.material->roughness = (float)column / (kColumnCount - 1);
			sphere.model.SetModel(sphereModel->Get());
			sphere.model.SetMaterial(sphere.material);
			sphere.model.SetWorldMatrix(Matrix4x4::MakeTranslation(position));
			sphere.model.SetBeReflected(false);
		}
	}

	LevelLoader::Load("Resources/scene.json", *Engine::GetGameObjectManager());

	std::shared_ptr<Texture> texture = Texture::Load("Resources/Sprite-0001.dds");

	sprite_.SetTexture(texture);
	sprite_.SetTexcoordRect({ 0.0f, 0.0f }, { 256.0f, 256.0f });
	sprite_.SetPosition({ 200.0f, 200.0f });
	sprite_.SetScale({ 200.0f, 200.0f });

	railCameraModel_.SetModel(AssetManager::GetInstance()->modelMap.Get("sphere")->Get());


	auto animationData = RailCameraSystem::AnimationLoader::LoadAnimation("Resources/RailCamera/railCamera.json");
	if (animationData) {
		railCameraController_ = std::make_unique<RailCameraSystem::RailCameraController>
			(
				std::make_shared<const RailCameraSystem::RailCameraAnimation>(*animationData)
			);
		railCameraController_->Play();
	}

	camera_ = std::make_shared<Camera>();

	flashlight_ = std::make_unique<Flashlight>();
	flashlight_->Initialize(&camera_->GetTransform(),camera_.get());
}

void TestScene::OnUpdate() {

	Engine::GetGameObjectManager()->Update();

	railCameraController_->Update(1.0f / 60.0f);
	auto transform = railCameraController_->GetCurrentTransform();
	transform = RailCameraSystem::RailCameraConverter::ConvertToLeftHand(transform);
	transform.UpdateMatrix();
	railCameraModel_.SetWorldMatrix(transform.worldMatrix);



	camera_->SetPosition(transform.translate);
	camera_->SetRotate(transform.rotate);
	camera_->UpdateMatrices();

	flashlight_->Update();

	railCameraController_->GetCurrentFrame();

#ifdef _DEBUG
	auto vertices = RailCameraSystem::RailCameraDebugUtils::CalculateFrustum(camera_->GetViewMatrix(), camera_->GetProjectionMatrix());

	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();

	Vector4 color = { 0.0f,1.0f,1.0f,1.0 };

	//近平面
	lineDrawer.AddLine(vertices[0], vertices[1], color); // 左下 -> 右下
	lineDrawer.AddLine(vertices[1], vertices[2], color); // 右下 -> 右上
	lineDrawer.AddLine(vertices[2], vertices[3], color); // 右上 -> 左上
	lineDrawer.AddLine(vertices[3], vertices[0], color); // 左上 -> 左下


	//遠平面
	lineDrawer.AddLine(vertices[4], vertices[5], color);
	lineDrawer.AddLine(vertices[5], vertices[6], color);
	lineDrawer.AddLine(vertices[6], vertices[7], color);
	lineDrawer.AddLine(vertices[7], vertices[4], color);

	//近平面と遠平面をつなぐ線
	lineDrawer.AddLine(vertices[0], vertices[4], color);
	lineDrawer.AddLine(vertices[1], vertices[5], color);
	lineDrawer.AddLine(vertices[2], vertices[6], color);
	lineDrawer.AddLine(vertices[3], vertices[7], color);
#endif // _DEBUG

	//
	RenderManager::GetInstance()->SetCamera(camera_);
}

void TestScene::OnFinalize() {

}
