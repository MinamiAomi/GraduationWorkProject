#include "Bats.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"

#include "Engine/Math/Camera.h"

const float Bats::batsFarLocate = 20.0f;

Bats::Bats(const std::vector<std::vector<bool>>& data)
{
	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("bat")->Get();
	radius_ = 5.0f;                       // Sphere用半径
	material_ = std::make_shared<Material>();

	isActive_ = true;

	uint32_t splitX = static_cast<uint32_t>(data[0].size());
	uint32_t splitY = static_cast<uint32_t>(data.size());   
	for (uint32_t r = 0; r < splitY; ++r) {
		for (uint32_t c = 0; c < splitX; ++c) {
			if (data[r][c]) {
				Vector3 pos = camera_->GetFrustumGridCenter(batsFarLocate, splitX, splitY, c, r);
				Emit(pos);
			}
		}
	}
}

Bats::Bats(const std::vector<std::vector<bool>>& data, const Camera& camera)
{
	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("bat")->Get();
	radius_ = 5.0f;                       // Sphere用半径
	material_ = std::make_shared<Material>();
	camera_ = &camera;

	isActive_ = true;

	uint32_t splitX = static_cast<uint32_t>(data[0].size());
	uint32_t splitY = static_cast<uint32_t>(data.size());
	for (uint32_t r = 0; r < splitY; ++r) {
		for (uint32_t c = 0; c < splitX; ++c) {
			if (data[r][c]) {
				Vector3 pos = camera_->GetFrustumGridCenter(batsFarLocate, splitX, splitY, c, r);
				Emit(pos);
			}
		}
	}
}

void Bats::Update()
{
	
	transform_.UpdateMatrix();

	for (auto it = bats_.begin(); it != bats_.end(); ) {

		Bat* p = it->get(); 


		p->goalTransform_.UpdateMatrix();
		p->transform_.translate = Vector3::Lerp(0.05f, p->transform_.translate, p->goalTransform_.worldMatrix.GetTranslate());

		if (p->transform_.scale.x <= 0.0f) {
			it = bats_.erase(it);
		}
		else {
			// 行列更新
			p->transform_.UpdateMatrix();
			p->modelInstance_.SetWorldMatrix(p->transform_.worldMatrix);

			++it;
		}

		if (p->collider_ &&
			!p->collider_->GetCollidedWith().empty()) {
			
		}

	}
	if (bats_.empty()) {
		isActive_ = false;
	}
}

void Bats::DebugDraw()
{
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
	lineDrawer.DrawSphere(transform_.worldMatrix.GetTranslate(), radius_, Vector4{ material_->albedo.x,material_->albedo.y,material_->albedo.z,1.0f});
}

void Bats::Emit(const Vector3& goalPos)
{
	auto newBat = std::make_unique<Bat>();

	newBat->modelInstance_.SetModel(model_);
	newBat->modelInstance_.SetMaterial(material_);

	newBat->collider_ = std::make_shared<SphereCollider>(CollisionCategory::ENEMY,
		CollisionCategory::FLASHLIGHT,
		Vector3::zero,
		model_->GetRadius());

	newBat->goalTransform_.SetParent(&camera_->GetTransform());
	newBat->collider_->SetParent(&newBat->transform_);

	newBat->goalTransform_.translate = goalPos - camera_->GetTransform().worldMatrix.GetTranslate();

	// 出現位置の計算（例としてランダムな球体内座標）
	Vector3 emitterWorldPos = transform_.worldMatrix.GetTranslate();
	Vector3 spawnPos = Vector3::zero;
	while (true) {
		Vector3 offset = {
			rnd_.NextFloatRange(-1.0f, 1.0f),
			rnd_.NextFloatRange(-1.0f, 1.0f),
			rnd_.NextFloatRange(-1.0f, 1.0f)
		};
		if (offset.LengthSquare() <= 1.0f) {
			spawnPos = emitterWorldPos + (offset * radius_);
			break;
		}
	}
	newBat->transform_.translate = spawnPos;


	bats_.push_back(std::move(newBat));
}
