#include "BatsBullets.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"
#include "Trolley.h"

#include "Engine/Math/Camera.h"

void BatsBullets::Initialize(const Camera& camera)
{
	emitShapeType_ = EmitShape::kSphere;
	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("Bullet")->Get();
	emitInterval_ = 60 * 10;
	emitTimer_ = 0;
	speed_ = 0.1f;
	material_ = std::make_shared<Material>();
	material_->emissive = { 1.0f,1.0f,1.0f };
	material_->emissiveIntensity = 10.0f;
	material_->albedo = Vector3{ 0.8f,0.15f,0.75f };

	camera_ = &camera;
}

void BatsBullets::Update()
{

	emitTimer_++;
	if (isEmit_) {
		emitTimer_++;
	}
	if ((emitTimer_ >= emitInterval_) && isEmit_) {
		for (int i = 0; i < 3; i++) {
			Emit(i);
		}
		emitTimer_ = 0;
	}
	
	for (auto it = particles_.begin(); it != particles_.end(); ) {

		Particle* p = it->get(); 

		p->transform_.translate.z += speed_;

		if (p->transform_.translate.z >= 20.0f) {
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

void BatsBullets::Emit(int num)
{
	auto newParticle = std::make_unique<Particle>();

	newParticle->modelInstance_.SetModel(model_);
	newParticle->modelInstance_.SetUseLighting(false);

	newParticle->transform_.SetParent(parentTransform_, false);
	newParticle->transform_.rotate = Quaternion::MakeForXAxis(Math::ToRadian * 90.0f);

	float scale = 0.5f - (float(num) * 0.2f);
	newParticle->transform_.scale = { scale, scale, scale };

	newParticle->modelInstance_.SetMaterial(material_);
	
	particles_.push_back(std::move(newParticle));
}
