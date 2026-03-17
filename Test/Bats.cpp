#include "Bats.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"

#include "Engine/Math/Camera.h"

#include "Trolley.h"

//const float Bats::batsFarLocate = 20.0f;

#ifdef _DEBUG
void Bats::Debug() {
	ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::TreeNode("Bats")) {

		ImGui::DragFloat("damage", &damage,0.1f,0.0f);
		ImGui::DragFloat("heal", &heal, 0.1f,0.0f);
		ImGui::DragFloat("batsFarLocate", &batsFarLocate, 0.1f,0.0f);
		ImGui::DragFloat("batteryDamage", &batteryDamage, 0.1f,0.0f);
		if (ImGui::Button("Save")) {
			JSON_OPEN("Resources/Data/GameScene/bats.json");
			JSON_OBJECT("bats");
			JSON_SAVE_BY_NAME("damage", damage);
			JSON_SAVE_BY_NAME("heal", heal);
			JSON_SAVE_BY_NAME("batsFarLocate", batsFarLocate);
			JSON_SAVE_BY_NAME("batteryDamage", batteryDamage);
			JSON_CLOSE();
		}
		ImGui::TreePop();
	}
	ImGui::End();
}
#endif // _DEBUG


Bats::Bats(const std::vector<std::vector<bool>>& data, const Camera& camera)
{



	JSON_OPEN("Resources/Data/GameScene/ghosts.json");
	JSON_OBJECT("ghosts");
	JSON_LOAD_BY_NAME("damage", damage);
	JSON_LOAD_BY_NAME("heal", heal);
	JSON_LOAD_BY_NAME("batsFarLocate", batsFarLocate);
	JSON_LOAD_BY_NAME("batteryDamage", batteryDamage);
	JSON_CLOSE();

	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("bat")->Get();
	animation_ = assetManager->animationMap.Get("batAnim");
    spawnSESound_ = assetManager->soundMap.Get("SE_BAT_SPAWN")->Get();
    deathSESound_ = assetManager->soundMap.Get("SE_BAT_DEATH")->Get();
    drainSESound_ = assetManager->soundMap.Get("SE_BAT_DRAIN")->Get();
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

		static const float ghostSpeed = 0.01f;

		p->goalTransform_.UpdateMatrix();
		p->goalT += ghostSpeed;
		if (p->goalT > 1.0f) {
			p->goalT = 1.0f;
			p->transform_.translate = p->goalTransform_.worldMatrix.GetTranslate();
		}
		else {
			p->transform_.translate = Vector3::Lerp(p->goalT, p->transform_.translate, p->goalTransform_.worldMatrix.GetTranslate());
		}
		p->transform_.rotate = Quaternion::MakeLookRotation(camera_->GetPosition()- p->transform_.translate);

		p->skeleton_->ApplyAnimation(animation_->Get()->GetAnimation("\u30a2\u30fc\u30de\u30c1\u30e5\u30a2Action"), p->animationTime_);
		p->skeleton_->Update();
		p->animationTime_ += 0.04f;
		if (p->animationTime_ >= 1.0f) {
			p->animationTime_ = 0.0f;
		}

		if (p->collider_ && !p->collider_->GetCollidedWith().empty()) {

			if (p->isDead_ == false) {
				// 揺れ時間を加算
				p->shakeTime_ += 1.0f / 60.0f;

				// サイン波で左右・上下に揺れる
				float shakeAmplitude = 0.15f;
				float shakeFreq = 30.0f;
				Vector3 right = p->transform_.rotate.GetRight();
				Vector3 up = p->transform_.rotate.GetUp();
				Vector3 shakeOffset =
					right * std::sin(p->shakeTime_ * shakeFreq) * shakeAmplitude +
					up * std::sin(p->shakeTime_ * shakeFreq * 1.3f + 1.0f) * shakeAmplitude * 0.6f;
				p->transform_.translate += shakeOffset;

				// スケールを脈動させて縮む感じ
				float scaleFreq = 15.0f;
				float scalePulse = 1.0f - 0.2f * std::abs(std::sin(p->shakeTime_ * scaleFreq));
				p->transform_.scale = Vector3(scalePulse, scalePulse, scalePulse);
			}
			p->hp_ -= damage;
			//ここで揺れる処理

		}
		else {
			
			p->particles_.SetIsEmit(false);
			//p->hp_ += heal;
		}

		p->hp_ = std::clamp(p->hp_, 0.0f, 1.0f);

		if (p->hp_ <= 0.0f && !p->isDead_) {
			// タイマーをカウントアップ
			p->deathTimer_ += 1.0f / 60.0f; // フレームレートに応じて調整

			float t = std::clamp(p->deathTimer_ / Bat::kDeathDuration, 0.0f, 1.0f);
			float scale = std::lerp(1.0f, 0.0f, t);
			p->transform_.scale = Vector3(scale, scale, scale);

			p->particles_.isDead_ = true;
			if (seCount_ < kSEMax && !p->isDeadSound ) {
				p->isDeadSound = true;
				auto as = std::make_shared<AudioSource>();
				(*as) = deathSESound_;
				as->SetVolume(0.7f);
				as->Play(false);
				playingAudioSourceList_.push_back(as);
				seCount_++;
			}

			if (t >= 1.0f) {
				p->isDead_ = true;
				p->particles_.isDead_ = true;


				if (!p->isAudioStop_) {
					p->isAudioStop_ = true;
					auto soundIt = std::find_if(playingAudioSourceList_.begin(), playingAudioSourceList_.end(), [this](const std::shared_ptr<AudioSource>& source) {
						return source->IsPlaying() && source->GetSound() == drainSESound_;
						});
					if (soundIt != playingAudioSourceList_.end()) {
						auto drainSound = *soundIt;
						if (drainSound->IsPlaying()) {
							drainSound->Stop();
							drainSound.reset();
						}
						playingAudioSourceList_.erase(soundIt);
					}

				}
				
			}
		}

		if (p->isDead_ == false) {
			//p->transform_.scale = Vector3(p->hp_, p->hp_, p->hp_);
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

	for (auto& audioSource : playingAudioSourceList_) {
		if (!audioSource->IsPlaying()) {
			audioSource.reset();
		}
	}
	playingAudioSourceList_.remove_if([](const std::shared_ptr<AudioSource>& source) {
		return source == nullptr;
        });

	if (bats_.empty() && playingAudioSourceList_.empty()) {
		isActive_ = false;
	}
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

	if (seCount_ < kSEMax) {
		auto as = std::make_shared<AudioSource>();
		(*as) = drainSESound_;
		as->SetVolume(0.5f);
		as->Play(true);
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

	newBat->goalT = 0.0f;

	bats_.push_back(newBat);
}
