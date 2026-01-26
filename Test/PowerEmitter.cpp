#include "PowerEmitter.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"
#include "Trolley.h"

bool PowerEmitter::isDebug = false;

void PowerEmitter::Initialize(EmitShape shape, const LightObject* parentLight)
{
	emitShapeType_ = shape;
	parentLight_ = parentLight;
	transform_.SetParent(parentLight->GetTransform(),false);
	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("Box")->Get();
	radius_ = parentLight_->GetModelResource()->GetRadius() * transform_.worldMatrix.GetScale().x;
	size_ = { 2.0f, 2.0f, 2.0f };
	emitInterval_ = 20;
	emitTimer_ = 0;
	minScale_ = 0.1f;
	maxScale_ = 0.2f;
	material_ = std::make_shared<Material>();
	material_->emissive = { 1.0f,1.0f,1.0f };
	material_->emissiveIntensity = 10.0f;
	const Color color = parentLight->GetColor();
	material_->albedo = Vector3{ color.GetR(),color.GetG(),color.GetB()};

	JSON_OPEN("Resources/Data/GameScene/powerEmitter.json");
	JSON_OBJECT("powerEmitter");
	JSON_LOAD_BY_NAME("minSpeed_", minSpeed_);
	JSON_LOAD_BY_NAME("maxSpeed_", maxSpeed_);
	JSON_LOAD_BY_NAME("minAngularVelocity_", minAngularVelocity_);
	JSON_LOAD_BY_NAME("maxAngularVelocity_", maxAngularVelocity_);
	JSON_LOAD_BY_NAME("scaleDecay_", scaleDecay_);
	JSON_LOAD_BY_NAME("emitInterval_", emitInterval_);
	JSON_LOAD_BY_NAME("minScale_", minScale_);
	JSON_LOAD_BY_NAME("maxScale_", maxScale_);
	JSON_CLOSE();
}

void PowerEmitter::Update()
{
	
	radius_ = parentLight_->GetModelResource()->GetRadius() * transform_.worldMatrix.GetScale().x;

#ifdef _DEBUG
	DebugDraw();
	if (!isDebug) {
		isDebug = true;
		isDebug_ = true;
	}
	if (isDebug_) {
		Debug();
	}
#endif // DEBUG_



	transform_.UpdateMatrix();
	if (parentLight_->GetIsActive() && parentLight_->GetHp()) {
		emitTimer_++;
		if (emitTimer_ >= emitInterval_) {
			Emit();
			emitTimer_ = 0;
		}
	}


	const auto& flashlight = Trolley::GetInstance()->GetFlashlight();
	
	for (auto it = particles_.begin(); it != particles_.end(); ) {

		Particle* p = it->get(); 

		if ((parentLight_->GetDamage() > 0.0f || !parentLight_->GetIsActive()) && !p->isSuction_) {
			p->isSuction_ = true;
		}

		if (p->isSuction_ == true) {
			p->transform_.translate = Vector3::Lerp(0.2f, p->transform_.translate, flashlight->GetTransform().worldMatrix.GetTranslate());
		}
		else {
			p->transform_.translate = p->transform_.translate + p->velocity_;

		}

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

void PowerEmitter::DebugDraw()
{
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();

	if (emitShapeType_ == EmitShape::kSphere) {
		lineDrawer.DrawSphere(transform_.worldMatrix.GetTranslate(), radius_, Vector4{ material_->albedo.x,material_->albedo.y,material_->albedo.z,1.0f});
	}
	else if (emitShapeType_ == EmitShape::kBox) {
		if (transform_.GetParent()) {
			Quaternion parentWorldRot = transform_.GetParent()->worldMatrix.GetRotate();
			lineDrawer.ObbDraw(transform_.worldMatrix.GetTranslate(), transform_.worldMatrix.GetScale(), parentWorldRot * transform_.rotate, Vector4{ material_->albedo.x,material_->albedo.y,material_->albedo.z,1.0f });
		}
		else {
			lineDrawer.ObbDraw(transform_.worldMatrix.GetTranslate(), size_,transform_.worldMatrix.GetRotate(), Vector4{ material_->albedo.x,material_->albedo.y,material_->albedo.z,1.0f});
		}
	}
}

void PowerEmitter::Debug()
{
	ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::TreeNode("PowerEmitter")) {

		ImGui::DragFloat("minSpeed_", &minSpeed_, 0.01f);
		ImGui::DragFloat("maxSpeed_", &maxSpeed_, 0.01f, minSpeed_);
		ImGui::DragFloat3("minAngularVelocity_", &minAngularVelocity_.x, 0.01f);
		ImGui::DragFloat3("maxAngularVelocity_", &maxAngularVelocity_.x, 0.01f);
		ImGui::DragFloat("scaleDecay_", &scaleDecay_, 0.01f);
		ImGui::DragInt("emitInterval_", &emitInterval_,1,0);
		ImGui::DragFloat("minScale_", &minScale_, 0.01f);
		ImGui::DragFloat("maxScale_", &maxScale_, 0.01f, minScale_);

		if (ImGui::Button("Save")) {
			JSON_OPEN("Resources/Data/GameScene/powerEmitter.json");
			JSON_OBJECT("powerEmitter");
			JSON_SAVE_BY_NAME("minSpeed_", minSpeed_);
			JSON_SAVE_BY_NAME("maxSpeed_", maxSpeed_);
			JSON_SAVE_BY_NAME("minAngularVelocity_", minAngularVelocity_);
			JSON_SAVE_BY_NAME("maxAngularVelocity_", maxAngularVelocity_);
			JSON_SAVE_BY_NAME("scaleDecay_", scaleDecay_);
			JSON_SAVE_BY_NAME("emitInterval_", emitInterval_);
			JSON_SAVE_BY_NAME("minScale_", minScale_);
			JSON_SAVE_BY_NAME("maxScale_", maxScale_);
			JSON_CLOSE();
		}
		ImGui::TreePop();

	}
	ImGui::End();
}

void PowerEmitter::Emit()
{
	auto newParticle = std::make_unique<Particle>();

	newParticle->modelInstance_.SetModel(model_);
	newParticle->modelInstance_.SetUseLighting(false);

	float startScale = rnd_.NextFloatRange(minScale_ , maxScale_ + (radius_ * 0.2f));
	newParticle->transform_.scale = { startScale, startScale, startScale };

	Vector3 randomRotEuler = {
		rnd_.NextFloatRange(0.0f, 6.28f),
		rnd_.NextFloatRange(0.0f, 6.28f),
		rnd_.NextFloatRange(0.0f, 6.28f)
	};
	newParticle->transform_.rotate = Quaternion::MakeFromEulerAngle(randomRotEuler);

	Vector3 spawnPos = { 0, 0, 0 };
	Vector3 emitterWorldPos = transform_.worldMatrix.GetTranslate();

	if (emitShapeType_ == EmitShape::kSphere) {
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
	}
	else if (emitShapeType_ == EmitShape::kBox) {
		Vector3 localPos;
		localPos.x = rnd_.NextFloatRange(-size_.x * 0.5f, size_.x * 0.5f);
		localPos.y = rnd_.NextFloatRange(-size_.y * 0.5f, size_.y * 0.5f);
		localPos.z = rnd_.NextFloatRange(-size_.z * 0.5f, size_.z * 0.5f);

		Vector3 rotatedOffset = transform_.worldMatrix.GetRotate() * localPos;
		spawnPos = emitterWorldPos + rotatedOffset;
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
	newParticle->modelInstance_.SetMaterial(material_);
	
	particles_.push_back(std::move(newParticle));
}
