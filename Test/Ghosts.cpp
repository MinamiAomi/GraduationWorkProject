#include "Ghosts.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"

#include "Engine/Math/Camera.h"

#include "Trolley.h"

#ifdef _DEBUG
void Ghosts::Debug() {
	ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::TreeNode("Ghosts")) {

		ImGui::DragInt("ghostAttackFrame", &ghostAttackFrame);
		ImGui::DragFloat("ghostAttackDamage", &ghostAttackDamage,0.1f);
		if (ImGui::Button("Save")) {
			JSON_OPEN("Resources/Data/GameScene/ghosts.json");
			JSON_OBJECT("ghosts");
			JSON_SAVE_BY_NAME("ghostAttackFrame", ghostAttackFrame);
			JSON_SAVE_BY_NAME("ghostAttackDamage", ghostAttackDamage);
			JSON_CLOSE();
		}
		ImGui::TreePop();
	}
	ImGui::End();
}
#endif // _DEBUG

Ghosts::Ghosts(const std::vector<std::vector<bool>>& data, const Camera& camera)
{

	JSON_OPEN("Resources/Data/GameScene/ghosts.json");
	JSON_OBJECT("ghosts");
	JSON_LOAD_BY_NAME("ghostAttackFrame", ghostAttackFrame);
	JSON_LOAD_BY_NAME("ghostAttackDamage", ghostAttackDamage);
	JSON_CLOSE();
	attackTime_ = 0;
	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("Ghost")->Get();
    spawnSESound_ = assetManager->soundMap.Get("SE_BOMB_GHOST_SPAWN")->Get();
    deathSESound_ = assetManager->soundMap.Get("SE_BOMB_GHOST_DEATH")->Get();
	explodeSESound_ = assetManager->soundMap.Get("SE_TROLLY_BURST")->Get();
	radius_ = 5.0f;                       // Sphere用半径
	camera_ = &camera;

	isActive_ = true;
	material_ = std::make_shared<Material>(model_->GetMaterials()[1]);
	material_->albedo = { 1.0f,1.0f,1.0f };
	uint32_t splitX = static_cast<uint32_t>(data[0].size());
	uint32_t splitY = static_cast<uint32_t>(data.size());

	Vector3 direction = {
		rnd_.NextFloatRange(-1.0f, 1.0f),
		rnd_.NextFloatRange(-1.0f, 1.0f),
		rnd_.NextFloatRange(-1.0f, 1.0f)
	};

	direction = direction.Normalized();

	//トロッコからどんくらい離れている場所からくるか
	const float offset = 10.0f;

	Vector3 spawnPos = Trolley::GetInstance()->GetTransform().worldMatrix.GetTranslate() + (direction * offset);
	transform_.translate = spawnPos;

	for (uint32_t r = 0; r < splitY; ++r) {
		for (uint32_t c = 0; c < splitX; ++c) {
			if (data[r][c]) {
				Vector3 pos = camera_->GetFrustumGridCenter(ghostsFarLocate, splitX, splitY, c, r);
				Emit(pos);
			}
		}
	}
	isExploded = false;
	
}

void Ghosts::Update()
{
	seCount_ = 0;
	transform_.UpdateMatrix();
	attackTime_++;
	material_->albedo = Vector3::Lerp(float(attackTime_) / float(ghostAttackFrame), { 1.0f,1.0f,1.0f }, goalColor_);
	if (attackTime_ >= ghostAttackFrame && !isExploded && ghost_.size() != 0) {
		float sumDamage = ghost_.size() * ghostAttackDamage;
		Trolley::GetInstance()->ghostDamage_ = sumDamage;
		isExploded = true;
		if (seCount_ < kSEMax) {
			auto as = std::make_shared<AudioSource>();
			(*as) = explodeSESound_;
			as->SetVolume(0.7f);
			as->Play(false);
			playingAudioSourceList_.push_back(as);
			seCount_++;
		}
	}
	for (auto it = ghost_.begin(); it != ghost_.end(); ) {
		auto p = *it;
		p->modelInstance_.SetColor(material_->albedo);
		p->sideStepTime_ += 0.05f;

		float horizontalAmplitude = 0.5f; 
		float verticalAmplitude = 0.3f;   

		float phaseOffset = (p->sideStepTime_ > 0.5f) ? 3.14159f : 0.0f;
		Vector3 sideOffset = p->transform_.rotate.GetRight() * std::sin(p->sideStepTime_ + phaseOffset) * horizontalAmplitude;

		Vector3 upOffset = p->transform_.rotate.GetUp() * std::sin(p->sideStepTime_ * 0.7f + 1.0f) * verticalAmplitude;

		Vector3 totalOffset = sideOffset + upOffset;

		static const float ghostSpeed = 0.01f;

		p->goalTransform_.UpdateMatrix();
		p->goalT += ghostSpeed;

		Vector3 targetPos = p->goalTransform_.worldMatrix.GetTranslate();

		if (p->goalT > 1.0f) {
			p->goalT = 1.0f;
			p->transform_.translate = targetPos + totalOffset;
		}
		else {
			Vector3 lerpPos = Vector3::Lerp(p->goalT, p->transform_.translate, targetPos);
			p->transform_.translate = lerpPos + totalOffset;
		}
		p->transform_.rotate = Quaternion::MakeLookRotation(camera_->GetPosition()- p->transform_.translate) * Quaternion::MakeForYAxis(180.0f * Math::ToRadian);

		if (p->collider_ && !p->collider_->GetCollidedWith().empty()) {

			float damage = 0.02f;
			if (p->isDead_ == false) {
				//p->particles_.SetIsHit(true);
			}
			p->hp_ -= damage;

		}
		else {
			
			//p->particles_.SetIsHit(false);
			float heal = 0.01f;
			p->hp_ += heal;
		}

		p->hp_ = std::clamp(p->hp_, 0.0f, 1.0f);
		if (attackTime_ >= ghostAttackFrame && !p->isDead_) {
			p->hp_ = 0.0f;
			p->particles_.SetIsHit(true);

		}
		if (p->hp_ <= 0.0f && !p->isDead_) {
			p->isDead_ = true;
			p->particles_.isDead_ = true;
			if (seCount_ < kSEMax) {
				auto as = std::make_shared<AudioSource>();
				(*as) = deathSESound_;
				as->SetVolume(0.7f);
				as->Play(false);
				playingAudioSourceList_.push_back(as);
				seCount_++;
			}
		}

		if (p->isDead_ == false) {
			p->transform_.scale = Vector3(p->hp_, p->hp_, p->hp_);
			p->particles_.SetIsEmit(true);
		}
		else {
			p->particles_.SetIsEmit(false);
			p->transform_.scale = Vector3(0.0f, 0.0f, 0.0f);
		}
	
		p->transform_.UpdateMatrix();
		p->modelInstance_.SetWorldMatrix(p->transform_.worldMatrix);
		p->particles_.Update();

		if (p->isDead_ && !p->particles_.HasParticles()) {
			it = ghost_.erase(it);
		}
		else {
			++it;
		}

	}
	if (ghost_.empty()) {
		isActive_ = false;
	}

	for (auto& audioSource : playingAudioSourceList_) {
		if (!audioSource->IsPlaying()) {
			audioSource.reset();
		}
	}
	playingAudioSourceList_.remove_if([](const std::shared_ptr<AudioSource>& source) {
		return source == nullptr;
        });
}

void Ghosts::DebugDraw()
{
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
	lineDrawer;

	for (auto it = ghost_.begin(); it != ghost_.end(); ) {

		Ghost* p = it->get();
		p->collider_->DrawDebug({1.0f,0.0f,0.0f,1.0f});
		it++;
	}
}

void Ghosts::Emit(const Vector3& goalPos)
{
	auto newGhost = std::make_shared<Ghost>();

	newGhost->modelInstance_.SetModel(model_);

	newGhost->modelInstance_.GetMaterials().emplace_back(std::make_shared<Material>(model_->GetMaterials()[0]));
	newGhost->modelInstance_.GetMaterials().emplace_back(material_);

	newGhost->particles_.Initialize(model_->GetRadius());
	newGhost->particles_.SetParent(&newGhost->transform_);

	if (seCount_ < kSEMax) {
		auto as = std::make_shared<AudioSource>();
		(*as) = spawnSESound_;
		as->SetVolume(0.7f);
		as->Play(false);
		playingAudioSourceList_.push_back(as);
		seCount_++;
	}

	newGhost->collider_ = std::make_shared<SphereCollider>(CollisionCategory::ENEMY,
		CollisionCategory::FLASHLIGHT,
		Vector3::zero,
		model_->GetRadius());

	newGhost->goalTransform_.SetParent(&camera_->GetTransform(),false);
	newGhost->collider_->SetParent(&newGhost->transform_);

	newGhost->goalTransform_.translate = goalPos - camera_->GetTransform().worldMatrix.GetTranslate();

	// 出現位置の計算（例としてランダムな球体内座標）
	Vector3 emitterWorldPos = transform_.worldMatrix.GetTranslate();
	Vector3 spawnPos = Vector3::zero;
	
	Vector3 direction = {
		rnd_.NextFloatRange(-1.0f, 1.0f),
		rnd_.NextFloatRange(-1.0f, 1.0f),
		rnd_.NextFloatRange(-1.0f, 1.0f)
	};
	
	direction = direction.Normalized();

	spawnPos = emitterWorldPos + (direction * radius_);
	
	newGhost->transform_.translate = spawnPos;

	newGhost->transform_.UpdateMatrix();

	newGhost->sideStepTime_ = rnd_.NextFloatRange(0.0f, 1.0f);

	newGhost->isDead_ = false;

	newGhost->hp_ = 1.0f;

	newGhost->goalT = 0.0f;

	ghost_.push_back(newGhost);
}
