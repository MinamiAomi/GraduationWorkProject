#include "ModelEmitter.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"

void ModelEmitter::Initialize(EmitShape shape)
{
	emitShapeType_ = shape;
	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("Box")->Get();
	radius_ = 1.0f;                       // Sphere用半径
	size_ = { 2.0f, 2.0f, 2.0f };         // Box用サイズ
	emitInterval_ = 1;
	emitTimer_ = 0;
	minScale_ = 1.0f;
	maxScale_ = 1.0f;
	material_ = std::make_shared<Material>();
	material_->emissive = { 1.0f,1.0f,1.0f };
	material_->emissiveIntensity = 10.0f;
	material_->albedo = { 1.0f,0.8f,0.0f };
}

void ModelEmitter::Update()
{
	
	transform_.UpdateMatrix();

	emitTimer_++;
	if (emitTimer_ >= emitInterval_) {
		Emit();
		emitTimer_ = 0;
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

void ModelEmitter::DebugDraw()
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

void ModelEmitter::Emit()
{
	auto newParticle = std::make_unique<Particle>();

	newParticle->modelInstance_.SetModel(model_);
	newParticle->modelInstance_.SetUseLighting(false);

	float startScale = rnd_.NextFloatRange(minScale_, maxScale_);
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

	newParticle->scaleSpeed_ = rnd_.NextFloatRange(minScaleDecay_, maxScaleDecay_);
	newParticle->modelInstance_.SetMaterial(material_);
	
	particles_.push_back(std::move(newParticle));
}
