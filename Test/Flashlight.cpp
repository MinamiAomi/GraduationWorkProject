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

	maxBattery_ = 100.0f;
	battery_ = maxBattery_;
	addBattery_ = 10.0f;
	subBattery_ = 0.1f;
	isLighting_ = true;

	collider_ = std::make_shared<ConeCollider>(
		CollisionCategory::FLASHLIGHT,
		(CollisionCategory::LIGHT | CollisionCategory::ENEMY | CollisionCategory::ITEM | CollisionCategory::PLAYER),
		Vector3::zero,
		0.0f, 0.0f,

		Quaternion::identity);
}

void Flashlight::Update()
{
#ifdef _DEBUG
	ImGui::Begin("LightBatter");

	ImGui::Checkbox("IsDebug", &isDebug_);

	ImGui::Separator();

	ImGui::VSliderFloat("##v_battery", ImVec2(40, 640), &battery_, 0.0f, maxBattery_, "%.1f");

	ImGui::SameLine();
	ImGui::Text("\n%.1f", battery_);

	ImGui::End();
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

			//ImGui::Separator();

			ImGui::DragFloat("Max", &maxBattery_, 1.0f);
			ImGui::DragFloat("Add", &addBattery_, 10.0f);
			ImGui::DragFloat("Sub", &subBattery_, 0.1f);
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

	UpdateCollision();
#ifdef _DEBUG
	// スポットライトの当たり判定描画
	//SpotLightDebugDraw();
#endif // _DEBUG

}

void Flashlight::UpdateCollision()
{

	Quaternion colliderRotation = lightTransform_.worldMatrix.GetRotate() * Quaternion::MakeForXAxis(-90.0f * Math::ToRadian);

	Vector3 heightOffset = colliderRotation * Vector3(0.0f, -lightRange_, 0.0f);

	collider_->quaternion = colliderRotation;
	collider_->height = lightRange_;
	collider_->center = lightTransform_.worldMatrix.GetTranslate() + heightOffset;

	collider_->radius = std::tan(fovAngle_ * 0.5f) * lightRange_;


	if (!collider_->GetCollidedWith().empty()) {
		for (const auto& collider : collider_->GetCollidedWith()) {
			collider;
			if (collider->categoryBits == CollisionCategory::LIGHT) {
				battery_ += addBattery_;
				battery_ = std::clamp(battery_, 0.0f, maxBattery_);
				isLighting_ = true;
			}
		}
	}
}

void Flashlight::UpdateLightPower()
{
	if (isLighting_) {
#ifdef _DEBUG
		if (!isDebug_) {
#endif // _DEBUG
			battery_ -= subBattery_;
			battery_ = std::clamp(battery_, 0.0f, maxBattery_);
			//バッテリーがなくなった場合
			if (battery_ <= 0.0f) {
				isLighting_ = false;
			}
#ifdef _DEBUG
		}
#endif // _DEBUG
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
