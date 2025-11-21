#include "Flashlight.h"

#include "Engine/Graphics/RenderManager.h"
#include "Engine/Graphics/GameWindow.h"
#include "Engine/Input/Input.h"

#include "Engine/Framework/AssetManager.h"

void Flashlight::Initialize(const Transform* parentTransform, const Camera* parentCamera)
{
	const Vector3 kFlashLightOffset = { 0.0f,0.0f,0.0f };

	parentTransform_ = parentTransform;
	parentCamera_ = parentCamera;
	transform_.SetParent(parentTransform, false);

	transform_.translate = kFlashLightOffset;
	transform_.UpdateMatrix();

	lightTransform_.SetParent(&transform_);
	lightTransform_.UpdateMatrix();
	auto assetManager = AssetManager::GetInstance();
	lightModel_.SetModel(assetManager->modelMap.Get("flashlight")->Get());
	lightModel_.SetWorldMatrix(lightTransform_.worldMatrix);

	maxLightPower_ = 100.0f;
	lightPower_ = maxLightPower_;
	addLightPower_ = 10.0f;
	subLightPower_ = 0.1f;
	isLighting_ = true;
}

void Flashlight::Update()
{
#ifdef _DEBUG
	ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::TreeNode("FlashLight")) {
		if (ImGui::TreeNode("Light")) {
			ImGui::DragFloat3("BaseTransform", &transform_.translate.x, 0.1f);

			ImGui::DragFloat("DistanceFromCamera", &distanceFromCamera_, 0.1f, 0.0f, 50.0f);
			Vector2 sphericalAnglesDegrees = { Math::ToDegree * sphericalAngleX_, Math::ToDegree * sphericalAngleY_ };
			ImGui::SliderFloat2("SphericalAngles", &sphericalAnglesDegrees.x, -90, 90);
			sphericalAngleX_ = Math::ToRadian * sphericalAnglesDegrees.x;
			sphericalAngleY_ = Math::ToRadian * sphericalAnglesDegrees.y;
			float fovAngleDegree = fovAngle_ * Math::ToDegree;
			ImGui::DragFloat("FovAngle", &fovAngleDegree, 1.0f, 1.0f, 90.0f);
			fovAngle_ = fovAngleDegree * Math::ToRadian;
			ImGui::SliderFloat("LightRange", &lightRange_, 1.0f, 100.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("LightPower")) {
			ImGui::SliderFloat("CurrentLightPower", &lightPower_, 0.0f, maxLightPower_, "Power: %.2f");

			ImGui::Separator();

			ImGui::DragFloat("Max", &maxLightPower_, 1.0f);
			ImGui::DragFloat("Add", &addLightPower_, 10.0f);
			ImGui::DragFloat("Sub", &subLightPower_, 0.1f);
			if (ImGui::Button("Add")) {
				lightPower_ += addLightPower_;
				lightPower_ = std::clamp(lightPower_, 0.0f, maxLightPower_);
			}
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
	ImGui::End();
	DebugMove();
#endif // _DEBUG

	transform_.UpdateMatrix();

	// ライトモデルの更新
	lightTransform_.rotate = Quaternion::MakeFromEulerAngle({ sphericalAngleY_, sphericalAngleX_, 0.0f });
	lightTransform_.translate = lightTransform_.rotate * (Vector3::unitZ * distanceFromCamera_);
	lightTransform_.UpdateMatrix();
	lightModel_.SetWorldMatrix(lightTransform_.worldMatrix);

	UpdateLightPower();
#ifdef _DEBUG
	// スポットライトの当たり判定描画
	SpotLightDebugDraw();
#endif // _DEBUG

}

void Flashlight::OnCollision()
{
	lightPower_ += addLightPower_;
	lightPower_ = std::clamp(lightPower_, 0.0f, maxLightPower_);
}

void Flashlight::UpdateLightPower()
{
	if (isLighting_) {
		lightPower_ -= subLightPower_;
		lightPower_ = std::clamp(lightPower_, 0.0f, maxLightPower_);
	}
}

void Flashlight::SpotLightDebugDraw() const
{
	const uint32_t segments = 16;
	std::vector<Vector3> vertices(segments);
	Vector3 apex = Vector3::zero * lightTransform_.worldMatrix;

	float halfFovAngle = fovAngle_ * 0.5f;
	float radius = lightRange_ * std::tan(halfFovAngle);

	for (uint32_t i = 0; i < segments; ++i) {
		float angle = (static_cast<float>(i) / segments) * Math::TwoPi;
		float x = radius * std::cos(angle);
		float y = radius * std::sin(angle);
		float z = lightRange_;
		vertices[i] = Vector3(x, y, z) * lightTransform_.worldMatrix;
	}

	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
	Vector4 color = { 1.0f, 1.0f, 0.0f, 1.0 };

	for (uint32_t i = 0; i < segments; ++i) {
		uint32_t  j = (i + 1) % segments;

		lineDrawer.AddLine(apex, vertices[i], color); // 側面
		lineDrawer.AddLine(vertices[i], vertices[j], color); // 底面
	}
}

void Flashlight::DebugMove() {
	const float anglerSpeed = 0.5f * Math::ToRadian;
	Input* input = Input::GetInstance();
	if (input->IsKeyPressed(DIK_W)) {
		sphericalAngleY_ -= anglerSpeed;
	}
	if (input->IsKeyPressed(DIK_S)) {
		sphericalAngleY_ += anglerSpeed;
	}
	if (input->IsKeyPressed(DIK_D)) {
		sphericalAngleX_ += anglerSpeed;
	}
	if (input->IsKeyPressed(DIK_A)) {
		sphericalAngleX_ -= anglerSpeed;
	}
}
