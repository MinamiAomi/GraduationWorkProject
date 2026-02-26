#include "ClearParticles.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/File/JsonConverter.h"


#ifdef _DEBUG
void ClearParticles::Debug()
{
	ImGui::Begin("TitleScene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::TreeNode("ClearParticles")) {


		ImGui::DragInt("deadFrame_", &deadFrame_, 1.0f, 0);
		ImGui::DragInt("emitNum_", &emitNum_, 1, 0);
		ImGui::DragFloat3("startScale_", &startScale_.x, 0.01f, 0.0f);
		ImGui::DragFloat("gravity_", &gravity_, 0.01f, 0.0f);
		ImGui::DragFloat3("size_", &size_.x, 0.01f);
		ImGui::DragFloat3("colorR_", &colorR_.x, 0.01f);
		ImGui::DragFloat3("colorG_", &colorG_.x, 0.01f);
		ImGui::DragFloat3("colorB_", &colorB_.x, 0.01f);
		ImGui::DragFloat3("position_", &position_.x, 0.01f);
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

		if (ImGui::Button("Save")) {
			JSON_OPEN("Resources/Data/GameScene/ClearParticles.json");
			JSON_OBJECT("clearParticle");
			JSON_SAVE_BY_NAME("deadFrame_", deadFrame_);
			JSON_SAVE_BY_NAME("emitNum_", emitNum_);
			JSON_SAVE_BY_NAME("startScale_", startScale_);
			JSON_SAVE_BY_NAME("gravity_", gravity_);
			JSON_SAVE_BY_NAME("size_", size_);
			JSON_SAVE_BY_NAME("colorR_", colorR_);
			JSON_SAVE_BY_NAME("colorG_", colorG_);
			JSON_SAVE_BY_NAME("colorB_", colorB_);
			JSON_SAVE_BY_NAME("position_", position_);
			JSON_SAVE_BY_NAME("minAngularVelocity_", minAngularVelocity_);
			JSON_SAVE_BY_NAME("maxAngularVelocity_", maxAngularVelocity_);
			JSON_CLOSE();
		}
		ImGui::TreePop();

	}
	ImGui::End();
}
#endif // _DEBUG

void ClearParticles::Initialize()
{
	particles_.clear();
	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("Box")->Get();

	JSON_OPEN("Resources/Data/GameScene/ClearParticles.json");
	JSON_OBJECT("clearParticle");
	JSON_LOAD_BY_NAME("deadFrame_", deadFrame_);
	JSON_LOAD_BY_NAME("emitNum_", emitNum_);
	JSON_LOAD_BY_NAME("startScale_", startScale_);
	JSON_LOAD_BY_NAME("gravity_", gravity_);
	JSON_LOAD_BY_NAME("size_", size_);
	JSON_LOAD_BY_NAME("colorR_", colorR_);
	JSON_LOAD_BY_NAME("colorG_", colorG_);
	JSON_LOAD_BY_NAME("colorB_", colorB_);
	JSON_LOAD_BY_NAME("position_", position_);
	JSON_LOAD_BY_NAME("minAngularVelocity_", minAngularVelocity_);
	JSON_LOAD_BY_NAME("maxAngularVelocity_", maxAngularVelocity_);
	JSON_CLOSE();
}

void ClearParticles::Update()
{
	transform_.translate = position_;
	transform_.UpdateMatrix();

	Emit();

	for (auto it = particles_.begin(); it != particles_.end(); ) {

		Particle* p = it->get();

		p->velocity_.y += gravity_;

		p->transform_.translate = p->transform_.translate + p->velocity_;

		Quaternion deltaRot = Quaternion::MakeFromEulerAngle(p->angularVelocity_);
		p->transform_.rotate = p->transform_.rotate * deltaRot;
		p->transform_.rotate = p->transform_.rotate.Normalized();


		p->deadFrame_--;

		if (p->deadFrame_ <= 0) {
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

void ClearParticles::DebugDraw()
{
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
	if (transform_.GetParent()) {
		Quaternion parentWorldRot = transform_.GetParent()->worldMatrix.GetRotate();
		lineDrawer.ObbDraw(transform_.worldMatrix.GetTranslate(), transform_.worldMatrix.GetScale(), parentWorldRot * transform_.rotate, Vector4{ 1.0f,1.0f,1.0f,1.0f });
	}
	else {
		lineDrawer.ObbDraw(transform_.worldMatrix.GetTranslate(), size_, transform_.worldMatrix.GetRotate(), Vector4{1.0f,1.0f,1.0f,1.0f });
	}
}


void ClearParticles::Emit()
{
	for (int i = 0; i < emitNum_; i++) {
		auto newParticle = std::make_unique<Particle>();

		newParticle->modelInstance_.SetModel(model_);

		newParticle->transform_.scale = startScale_;

		Vector3 randomRotEuler = {
			rnd_.NextFloatRange(0.0f, 6.28f),
			rnd_.NextFloatRange(0.0f, 6.28f),
			rnd_.NextFloatRange(0.0f, 6.28f)
		};
		newParticle->transform_.rotate = Quaternion::MakeFromEulerAngle(randomRotEuler);


		Vector3 spawnPos = { 0, 0, 0 };
		Vector3 emitterWorldPos = transform_.worldMatrix.GetTranslate();

		
		Vector3 localPos;
		localPos.x = rnd_.NextFloatRange(-size_.x * 0.5f, size_.x * 0.5f);
		localPos.y = rnd_.NextFloatRange(-size_.y * 0.5f, size_.y * 0.5f);
		localPos.z = rnd_.NextFloatRange(-size_.z * 0.5f, size_.z * 0.5f);

		Vector3 rotatedOffset = transform_.worldMatrix.GetRotate() * localPos;
		spawnPos = emitterWorldPos + rotatedOffset;

		newParticle->transform_.translate = spawnPos;

		newParticle->velocity_ = Vector3::zero;

		newParticle->angularVelocity_ = {
		rnd_.NextFloatRange(minAngularVelocity_.x, maxAngularVelocity_.x),
		rnd_.NextFloatRange(minAngularVelocity_.y, maxAngularVelocity_.y),
		rnd_.NextFloatRange(minAngularVelocity_.z, maxAngularVelocity_.z)
		};

		newParticle->material_ = std::make_shared<Material>();
		newParticle->material_->emissive = { 1.0f,1.0f,1.0f };
		newParticle->material_->emissiveIntensity = 10.0f;

		//0か1か2が出てくる
		int random = rnd_.NextIntRange(0, 2);
		Vector3 color;
		switch (random) {
		case 0: color = colorR_; break;
		case 1: color = colorG_; break;
		case 2: color = colorB_; break;
		}

		newParticle->material_->albedo = color;

		newParticle->modelInstance_.GetMaterials().emplace_back(newParticle->material_);
		newParticle->deadFrame_ = deadFrame_;

		particles_.push_back(std::move(newParticle));
	}
}
