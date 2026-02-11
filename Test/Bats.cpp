#include "Bats.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"

#include "Engine/Math/Camera.h"

#include "Trolley.h"

//const float Bats::batsFarLocate = 20.0f;

Bats::Bats(const std::vector<std::vector<bool>>& data, const Camera& camera)
{
	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("bat")->Get();
	animation_ = assetManager->animationMap.Get("batAnim");
    spawnSESound_ = assetManager->soundMap.Get("SE_BAT_SPAWN")->Get();
    deathSESound_ = assetManager->soundMap.Get("SE_BAT_DEATH")->Get();
	radius_ = 5.0f;                       // Sphere用半径
	camera_ = &camera;

	isActive_ = true;

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
				Vector3 pos = camera_->GetFrustumGridCenter(batsFarLocate, splitX, splitY, c, r);
				Emit(pos);
			}
		}
	}
}

void Bats::Update()
{
	seCount_ = 0;
	
	transform_.UpdateMatrix();

	for (auto it = bats_.begin(); it != bats_.end(); ) {

		auto p = *it; 
		p->sideStepTime_ += 0.05f;

		//float amplitude = 0.5f;
		//float phaseOffset = (p->sideStepTime_ > 0.5f) ? 3.14159f : 0.0f;
		//Vector3 sideOffset = p->transform_.rotate.GetRight() * std::sin(p->sideStepTime_ + phaseOffset) * amplitude;

		p->goalTransform_.UpdateMatrix();
		p->transform_.translate = Vector3::Lerp(0.3f, p->transform_.translate, p->goalTransform_.worldMatrix.GetTranslate());
		p->transform_.rotate = Quaternion::MakeLookRotation(camera_->GetPosition()- p->transform_.translate);

		p->skeleton_->ApplyAnimation(animation_->Get()->GetAnimation("\u30a2\u30fc\u30de\u30c1\u30e5\u30a2Action"), p->animationTime_);
		p->skeleton_->Update();
		p->animationTime_ += 0.04f;
		if (p->animationTime_ >= 1.0f) {
			p->animationTime_ = 0.0f;
		}

		if (p->collider_ && !p->collider_->GetCollidedWith().empty()) {

			float damage = 0.02f;
			if (p->isDead_ == false) {
				p->particles_.SetIsEmit(true);
			}
			p->hp_ -= damage;

		}
		else {
			
			p->particles_.SetIsEmit(false);
			float heal = 0.01f;
			p->hp_ += heal;
		}

		p->hp_ = std::clamp(p->hp_, 0.0f, 1.0f);

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
		}
		else {
			p->particles_.SetIsEmit(false);
			//p->bullets_.SetIsEmit(false);
			p->transform_.scale = Vector3(0.0f, 0.0f, 0.0f);
		}
		p->particles_.emitInterval_ = static_cast<int>(std::lerp(0, 11, p->hp_));
		p->particles_.scaleDecay_ = std::lerp(p->particles_.goalScaleDecay_, p->particles_.startScaleDecay_, p->hp_);
		p->transform_.UpdateMatrix();
		p->modelInstance_.SetWorldMatrix(p->transform_.worldMatrix);
		p->particles_.Update();
		//p->bullets_.Update();

		if (p->isDead_ && !p->particles_.HasParticles()/* && !p->bullets_.HasParticles()*/) {
			it = bats_.erase(it);
		}
		else {
			++it;
		}

	}
	if (bats_.empty()) {
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

void Bats::DebugDraw()
{
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
	lineDrawer;

	for (auto it = bats_.begin(); it != bats_.end(); ) {

		Bat* p = it->get();
		p->collider_->DrawDebug({1.0f,0.0f,0.0f,1.0f});
		it++;
	}
}

void Bats::Emit(const Vector3& goalPos)
{
	auto newBat = std::make_shared<Bat>();

	newBat->modelInstance_.SetModel(model_);
	newBat->skeleton_ = std::make_unique<Skeleton>();
	newBat->skeleton_->Create(model_);
	newBat->modelInstance_.SetSkeleton(newBat->skeleton_);
	
	newBat->particles_.Initialize(model_->GetRadius());
	newBat->particles_.SetParent(&newBat->transform_);

	if (seCount_ < kSEMax) {
		auto as = std::make_shared<AudioSource>();
		(*as) = spawnSESound_;
		as->SetVolume(0.7f);
		as->Play(false);
		playingAudioSourceList_.push_back(as);
		seCount_++;
	}

	//newBat->bullets_.Initialize(*camera_);
	//newBat->bullets_.SetBatTransform(newBat->transform_);
	//newBat->bullets_.SetIsEmit(true);

	newBat->collider_ = std::make_shared<SphereCollider>(CollisionCategory::ENEMY,
		CollisionCategory::FLASHLIGHT,
		Vector3::zero,
		model_->GetRadius());

	newBat->goalTransform_.SetParent(&camera_->GetTransform(),false);
	newBat->collider_->SetParent(&newBat->transform_);

	newBat->goalTransform_.translate = goalPos - camera_->GetTransform().worldMatrix.GetTranslate();

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
	
	newBat->transform_.translate = spawnPos;

	newBat->transform_.UpdateMatrix();

	newBat->animationTime_ = rnd_.NextFloatRange(0.0f, 1.0f);

	newBat->sideStepTime_ = rnd_.NextFloatRange(0.0f, 1.0f);

	newBat->isDead_ = false;

	newBat->hp_ = 1.0f;

	bats_.push_back(newBat);
}
