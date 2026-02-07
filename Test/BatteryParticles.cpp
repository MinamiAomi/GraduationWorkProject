#include "BatteryParticles.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"
#include "Trolley.h"



void BatteryParticles::Initialize(const Transform* transform , const BatsManager* batsManager)
{
	transform_.SetParent(transform, false);
	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("Box")->Get();
	radius_ = Trolley::GetInstance()->GetBatteryRadius();
	emitTimer_ = 0;
	minScale_ = 0.1f;
	maxScale_ = 0.2f;
	material_ = std::make_shared<Material>();
	material_->emissive = { 1.0f,1.0f,1.0f };
	material_->emissiveIntensity = 10.0f;
	
	batsManager_ = batsManager;

	JSON_OPEN("Resources/Data/GameScene/batteryParticles.json");
	JSON_OBJECT("batteryParticles");
	JSON_LOAD_BY_NAME("minSpeed_", minSpeed_);
	JSON_LOAD_BY_NAME("maxSpeed_", maxSpeed_);
	JSON_LOAD_BY_NAME("minAngularVelocity_", minAngularVelocity_);
	JSON_LOAD_BY_NAME("maxAngularVelocity_", maxAngularVelocity_);
	JSON_LOAD_BY_NAME("scaleDecay_", scaleDecay_);
	JSON_LOAD_BY_NAME("emitInterval_", emitInterval_);
	JSON_LOAD_BY_NAME("minScale_", minScale_);
	JSON_LOAD_BY_NAME("maxScale_", maxScale_);
	JSON_LOAD_BY_NAME("color_", color_);
	JSON_LOAD_BY_NAME("toBatSpeed_", toBatSpeed_);
	JSON_LOAD_BY_NAME("toBatScaleSpeed_", toBatScaleSpeed_);
	JSON_CLOSE();
	material_->albedo = color_;
}

void BatteryParticles::Update()
{
	material_->albedo = color_;
	transform_.UpdateMatrix();
	if (Trolley::GetInstance()->GetIsHitFlashlight()) {
		emitTimer_++;
		if (emitTimer_ >= emitInterval_) {
			Emit();
			emitTimer_ = 0;
		}
	}

	for (auto it = particles_.begin(); it != particles_.end(); ) {

		Particle* p = it->get();

		p->transform_.translate = p->transform_.translate + p->velocity_;
		
		Quaternion deltaRot = Quaternion::MakeFromEulerAngle(p->angularVelocity_);
		p->transform_.rotate = p->transform_.rotate * deltaRot;
		p->transform_.rotate = p->transform_.rotate.Normalized();

		p->transform_.scale.x -= p->scaleSpeed_;
		p->transform_.scale.y -= p->scaleSpeed_;
		p->transform_.scale.z -= p->scaleSpeed_;

		if (p->transform_.scale.x <= 0.0f) {
			it = particles_.erase(it);
		}
		else {
			// 行列更新
			p->transform_.UpdateMatrix();
			p->modelInstance_.SetWorldMatrix(p->transform_.worldMatrix);

			++it;
		}
	}
}

void BatteryParticles::DebugDraw()
{
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();

	
	lineDrawer.DrawSphere(transform_.worldMatrix.GetTranslate(), radius_, Vector4{ material_->albedo.x,material_->albedo.y,material_->albedo.z,1.0f });
	
}

void BatteryParticles::Debug()
{
	ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::TreeNode("BatteryParticles")) {

		ImGui::DragFloat("minSpeed_", &minSpeed_, 0.000000001f,0.0f,1.0f,"%.10f");
		ImGui::DragFloat("maxSpeed_", &maxSpeed_, 0.000000001f, minSpeed_, 1.0f, "%.10f");
		if (minSpeed_ > maxSpeed_) {
			maxSpeed_ = minSpeed_ + 0.00001f;
		}
		ImGui::DragFloat3("minAngularVelocity_", &minAngularVelocity_.x, 0.0000001f, 0.0f, 1.0f, "%.10f");
		ImGui::DragFloat3("maxAngularVelocity_", &maxAngularVelocity_.x, 0.0000001f, 0.0f, 1.0f, "%.10f");
		if (minAngularVelocity_.x > maxAngularVelocity_.x) {
			maxAngularVelocity_.x = minAngularVelocity_.x + 0.0000001f;
		}
		if (minAngularVelocity_.y > maxAngularVelocity_.y) {
			maxAngularVelocity_.y = minAngularVelocity_.y + 0.0000001f;
		}
		if (minAngularVelocity_.z > maxAngularVelocity_.z) {
			maxAngularVelocity_.z = minAngularVelocity_.z + 0.0000001f;
		}

		ImGui::DragFloat("scaleDecay_", &scaleDecay_, 0.0000001f, 0.0f, 1.0f, "%.10f");
		if (scaleDecay_ < 0.00000001f) {
			scaleDecay_ = 0.00000001f;
		}
		ImGui::DragInt("emitInterval_", &emitInterval_, 1, 0);
		ImGui::DragFloat("minScale_", &minScale_, 0.0000001f, 0.0f, 1.0f, "%.10f");
		ImGui::DragFloat("maxScale_", &maxScale_, 0.0000001f, minScale_,1.0f, "%.10f");
		if (minScale_ > maxScale_) {
			maxScale_ = minScale_ + 0.0000001f;
		}
		ImGui::DragFloat3("color_", &color_.x, 0.01f);
		ImGui::DragFloat("toBatSpeed_", &toBatSpeed_, 0.0001f, 0.0f, 1.0f, "%.10f");
		ImGui::DragFloat("toBatScaleSpeed_", &toBatScaleSpeed_, 0.0001f, 0.0f, 1.0f, "%.10f");
		if (ImGui::Button("Save")) {
			JSON_OPEN("Resources/Data/GameScene/batteryParticles.json");
			JSON_OBJECT("batteryParticles");
			JSON_SAVE_BY_NAME("minSpeed_", minSpeed_);
			JSON_SAVE_BY_NAME("maxSpeed_", maxSpeed_);
			JSON_SAVE_BY_NAME("minAngularVelocity_", minAngularVelocity_);
			JSON_SAVE_BY_NAME("maxAngularVelocity_", maxAngularVelocity_);
			JSON_SAVE_BY_NAME("scaleDecay_", scaleDecay_);
			JSON_SAVE_BY_NAME("emitInterval_", emitInterval_);
			JSON_SAVE_BY_NAME("minScale_", minScale_);
			JSON_SAVE_BY_NAME("maxScale_", maxScale_);
			JSON_SAVE_BY_NAME("color_", color_);
			JSON_SAVE_BY_NAME("toBatSpeed_", toBatSpeed_);
			JSON_SAVE_BY_NAME("toBatScaleSpeed_", toBatScaleSpeed_);
			JSON_CLOSE();
		}
		ImGui::TreePop();

	}
	ImGui::End();
}

void BatteryParticles::Emit()
{
	auto newParticle = std::make_unique<Particle>();

	newParticle->modelInstance_.SetModel(model_);
	newParticle->modelInstance_.SetUseLighting(false);

	newParticle->transform_.SetParent(&transform_, false);

	float startScale = rnd_.NextFloatRange(minScale_, maxScale_ + (radius_ * 0.2f));
	newParticle->transform_.scale = { startScale, startScale, startScale };

	Vector3 randomRotEuler = {
		rnd_.NextFloatRange(0.0f, 6.28f),
		rnd_.NextFloatRange(0.0f, 6.28f),
		rnd_.NextFloatRange(0.0f, 6.28f)
	};
	newParticle->transform_.rotate = Quaternion::MakeFromEulerAngle(randomRotEuler);

	Vector3 spawnPos = { 0, 0, 0 };

	while (true) {
		Vector3 offset = {
			rnd_.NextFloatRange(-1.0f, 1.0f),
			rnd_.NextFloatRange(-1.0f, 1.0f),
			rnd_.NextFloatRange(-1.0f, 1.0f)
		};
		if (offset.LengthSquare() <= 1.0f) {
			spawnPos = offset * radius_;
			break;
		}
	}


	newParticle->transform_.translate = spawnPos;

	Vector3 randomDir = {
		rnd_.NextFloatRange(-1.0f, 1.0f),
		rnd_.NextFloatRange(-1.0f, 1.0f),
		rnd_.NextFloatRange(-1.0f, 1.0f)
	};
	if (randomDir.LengthSquare() != 0) {
		randomDir = randomDir.Normalized();
	}

	float speed = rnd_.NextFloatRange(minSpeed_, maxSpeed_);
	newParticle->velocity_ = randomDir * speed;

	newParticle->angularVelocity_ = {
		rnd_.NextFloatRange(minAngularVelocity_.x, maxAngularVelocity_.x),
		rnd_.NextFloatRange(minAngularVelocity_.y, maxAngularVelocity_.y),
		rnd_.NextFloatRange(minAngularVelocity_.z, maxAngularVelocity_.z)
	};

	newParticle->scaleSpeed_ = scaleDecay_;
	newParticle->modelInstance_.GetMaterials().emplace_back(material_);

	particles_.push_back(std::move(newParticle));
}


void BatteryParticles::ToBatEmit()
{
	auto newParticle = std::make_unique<Particle>();

	newParticle->modelInstance_.SetModel(model_);
	newParticle->modelInstance_.SetUseLighting(false);

	newParticle->transform_.SetParent(&transform_, false);

	newParticle->transform_.scale = { maxScale_, maxScale_, maxScale_ };

	Vector3 randomRotEuler = {
		rnd_.NextFloatRange(0.0f, 6.28f),
		rnd_.NextFloatRange(0.0f, 6.28f),
		rnd_.NextFloatRange(0.0f, 6.28f)
	};
	newParticle->transform_.rotate = Quaternion::MakeFromEulerAngle(randomRotEuler);

	Vector3 spawnPos = { 0, 0, 0 };

	while (true) {
		Vector3 offset = {
			rnd_.NextFloatRange(-1.0f, 1.0f),
			rnd_.NextFloatRange(-1.0f, 1.0f),
			rnd_.NextFloatRange(-1.0f, 1.0f)
		};
		if (offset.LengthSquare() <= 1.0f) {
			spawnPos = offset * radius_;
			break;
		}
	}

	newParticle->transform_.translate = spawnPos;

	newParticle->transform_.UpdateMatrix();
	
	Vector3 direction = Vector3(batsManager_->GetRandomBatPosition() - newParticle->transform_.worldMatrix.GetTranslate()).Normalize();
	newParticle->velocity_ = direction * toBatSpeed_;

	newParticle->angularVelocity_ = {
		rnd_.NextFloatRange(minAngularVelocity_.x, maxAngularVelocity_.x),
		rnd_.NextFloatRange(minAngularVelocity_.y, maxAngularVelocity_.y),
		rnd_.NextFloatRange(minAngularVelocity_.z, maxAngularVelocity_.z)
	};

	newParticle->scaleSpeed_ = scaleDecay_;
	newParticle->modelInstance_.GetMaterials().emplace_back(material_);

	particles_.push_back(std::move(newParticle));
}
