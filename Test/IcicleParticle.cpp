#include "IcicleParticle.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/File/JsonConverter.h"


#ifdef _DEBUG
void IcicleParticle::Debug()
{
	ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::TreeNode("IcicleParticle")) {

		ImGui::DragFloat("minSpeed_", &minSpeed_, 0.01f);
		ImGui::DragFloat("maxSpeed_", &maxSpeed_, 0.01f, minSpeed_);
		if (minSpeed_ > maxSpeed_) {
			maxSpeed_ = minSpeed_ + 0.01f;
		}
		ImGui::DragFloat3("minAngularVelocity_", &minAngularVelocity_.x, 0.01f);
		ImGui::DragFloat3("maxAngularVelocity_", &maxAngularVelocity_.x, 0.01f);
		if (minAngularVelocity_.x > maxAngularVelocity_.x) {
			maxAngularVelocity_.x = minAngularVelocity_.x + 0.01f;
		}
		if (minAngularVelocity_.y > maxAngularVelocity_.y) {
			maxAngularVelocity_.y = minAngularVelocity_.y + 0.01f;
		}
		if (minAngularVelocity_.z > maxAngularVelocity_.z) {
			maxAngularVelocity_.z = minAngularVelocity_.z + 0.01f;
		}

		ImGui::DragFloat("scaleDecay_", &scaleDecay_, 0.0001f, 0.0001f);
		if (scaleDecay_ < 0.0001f) {
			scaleDecay_ = 0.0001f;
		}
		ImGui::DragInt("emitNum_", &emitNum_, 1, 0);
		ImGui::DragFloat("startScale_", &startScale_, 0.01f,0.0f);
		ImGui::DragFloat("gravity_", &gravity_, 0.01f, 0.0f);
		ImGui::DragFloat3("color_", &color_.x, 0.01f);
	
		if (ImGui::Button("Save")) {
			JSON_OPEN("Resources/Data/GameScene/IcicleParticle.json");
			JSON_OBJECT("icicleParticle");
			JSON_SAVE_BY_NAME("minSpeed_", minSpeed_);
			JSON_SAVE_BY_NAME("maxSpeed_", maxSpeed_);
			JSON_SAVE_BY_NAME("minAngularVelocity_", minAngularVelocity_);
			JSON_SAVE_BY_NAME("maxAngularVelocity_", maxAngularVelocity_);
			JSON_SAVE_BY_NAME("scaleDecay_", scaleDecay_);
			JSON_SAVE_BY_NAME("emitNum_", emitNum_);
			JSON_SAVE_BY_NAME("startScale_", startScale_);
			JSON_SAVE_BY_NAME("gravity_", gravity_);
			JSON_SAVE_BY_NAME("color_", color_);
			JSON_CLOSE();
		}
		ImGui::TreePop();

	}
	ImGui::End();
}
#endif // _DEBUG

void IcicleParticle::Initialize()
{
	particles_.clear();
	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("IcicleDebris")->Get();
	/*material_ = std::make_shared<Material>();
	material_->emissive = { 1.0f,1.0f,1.0f };
	material_->emissiveIntensity = 10.0f;*/

	JSON_OPEN("Resources/Data/GameScene/IcicleParticle.json");
	JSON_OBJECT("icicleParticle");
	JSON_LOAD_BY_NAME("minSpeed_", minSpeed_);
	JSON_LOAD_BY_NAME("maxSpeed_", maxSpeed_);
	JSON_LOAD_BY_NAME("minAngularVelocity_", minAngularVelocity_);
	JSON_LOAD_BY_NAME("maxAngularVelocity_", maxAngularVelocity_);
	JSON_LOAD_BY_NAME("scaleDecay_", scaleDecay_);
	JSON_LOAD_BY_NAME("emitNum_", emitNum_);
	JSON_LOAD_BY_NAME("startScale_", startScale_);
	JSON_LOAD_BY_NAME("gravity_", gravity_);
	JSON_LOAD_BY_NAME("color_", color_);
	JSON_CLOSE();
	//material_->albedo = color_;
}

void IcicleParticle::Update()
{

	transform_.UpdateMatrix();
	//material_->albedo = color_;

	for (auto it = particles_.begin(); it != particles_.end(); ) {

		Particle* p = it->get();

		p->velocity_.y += gravity_;

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

void IcicleParticle::Emit(const Vector3& position)
{
	for (int i = 0; i < emitNum_; i++) {
		auto newParticle = std::make_unique<Particle>();

		newParticle->modelInstance_.SetModel(model_);
		newParticle->transform_.scale = { startScale_, startScale_, startScale_ };

		Vector3 randomRotEuler = {
			rnd_.NextFloatRange(0.0f, 6.28f),
			rnd_.NextFloatRange(0.0f, 6.28f),
			rnd_.NextFloatRange(0.0f, 6.28f)
		};
		newParticle->transform_.rotate = Quaternion::MakeFromEulerAngle(randomRotEuler);

	
		newParticle->transform_.translate = position;

		Vector3 randomDir = {
			rnd_.NextFloatRange(-0.5f, 0.5f),
			rnd_.NextFloatRange(0.0f, 1.0f),
			rnd_.NextFloatRange(-0.5f, 0.5f)
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
		//newParticle->modelInstance_.GetMaterials().emplace_back(material_);

		particles_.push_back(std::move(newParticle));
	}
}

void IcicleParticle::Emit()
{
	for (int i = 0; i < emitNum_; i++) {
		auto newParticle = std::make_unique<Particle>();

		newParticle->modelInstance_.SetModel(model_);
		newParticle->modelInstance_.SetUseLighting(false);

		newParticle->transform_.scale = { startScale_, startScale_, startScale_ };

		Vector3 randomRotEuler = {
			rnd_.NextFloatRange(0.0f, 6.28f),
			rnd_.NextFloatRange(0.0f, 6.28f),
			rnd_.NextFloatRange(0.0f, 6.28f)
		};
		newParticle->transform_.rotate = Quaternion::MakeFromEulerAngle(randomRotEuler);


		newParticle->transform_.translate = transform_.worldMatrix.GetTranslate();

		Vector3 randomDir = {
			rnd_.NextFloatRange(-0.5f, 0.5f),
			rnd_.NextFloatRange(0.0f, 1.0f),
			rnd_.NextFloatRange(-0.5f, 0.5f)
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
		//newParticle->modelInstance_.GetMaterials().emplace_back(material_);

		particles_.push_back(std::move(newParticle));
	}
}
